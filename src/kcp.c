//
// Created by YuQiang on 2017-09-21.
//

#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include "3rd/ikcp/ikcp.h"

#include "inc.h"
#include "log.h"
#include "event.h"
#include "auth.h"
#include "kcp.h"
#include "util.h"

#ifdef DEBUG_KCP
#define debugKcp(s...) __LOG(stdout, DEBUG_KCP, s)
#else
#define debugKcp(s...)
#endif

static struct {
    int isServer;
    // kcp global config
    sectun_kcp_config_t config;
    //databuferr
    char dataBuffer[DATA_BUFFER_SIZE];
    // kcp timer used to do update
    uev_t timerEvent;
} _kcpCtx;

// kcp interface
static struct itransport _kcpTransport;
static int _isKcpInit = 0;

/**
 * kcp output func to do data send
 *
 * @param buf
 * @param len
 * @param kcp
 * @param user
 * @return
 */
static int kcpOutput(const char *buf, int len, ikcpcb *kcp, void *user) {
    debugKcp("kcp output [%d] bytes {%s}", len, log_hex_memory_32_bytes(buf));
    return _kcpTransport.forwardWrite((char *) buf, len, user);
}

/**
 *  return current time in million seconds
 * @return
 */
static IUINT32 getClock() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // caculate milliseconds
    return (IUINT32) milliseconds;
}

/**
 * update client kcp
 * @param client
 */
static void kcpUpdateClientKcp(client_info_t *client) {
    if (0 != client->conv && NULL != client->pKcp) {
        ikcp_update(client->pKcp, getClock());
    }
}

/**
 *  do kcp update every time
 *
 * @param w
 * @param arg
 * @param events
 */
void timerKcpUpdate(uev_t *w, void *arg, int events) {
    // do kcp update
    sectunAuthIterateClientArray(kcpUpdateClientKcp);
}

/**
 *  创建一个 kcp 链接
 *
 *
 * @param conv
 * @return
 */
static ikcpcb *kcpCreateConv(IUINT32 conv, void *context) {

    assert(NULL != context);

    client_info_t *client = (client_info_t *) context;

    assert(NULL == client->pKcp);

    // save conv
    client->conv = conv;
    client->pKcp = ikcp_create(client->conv, context);

    if (NULL == client->pKcp) {
        errf("can not create ikcp for user token [%.*s] tunip [%s]",
             AUTH_USERTOKEN_LEN, client->userToken, ipToString(client->tunIp));
        return NULL;
    }

    // make sure the config is correct, we set interval to 10 as minimum, 100 as max
    if (_kcpCtx.config.interval < 10 || _kcpCtx.config.interval > 100) {
        _kcpCtx.config.interval = 10;
    }

    // do kcp config
    ikcp_nodelay(client->pKcp, _kcpCtx.config.nodelay, _kcpCtx.config.interval,
                 _kcpCtx.config.resend, _kcpCtx.config.nc);
    ikcp_wndsize(client->pKcp, _kcpCtx.config.sndwnd, _kcpCtx.config.rcvwnd);
    ikcp_setmtu(client->pKcp, _kcpCtx.config.mtu);

    // set output function
    ikcp_setoutput(client->pKcp, kcpOutput);

    // extra paramter
    //_pKcp->rx_minrto = 10;
    //_pKcp->fastresend = 1;

    return client->pKcp;
}

/**
 *  stop kcp connection
 */
static void kcpStopConv(void *context) {

    assert(NULL != context);

    client_info_t *client = (client_info_t *) context;

    assert(NULL != client->pKcp);

    // release kcp object
    ikcp_release(client->pKcp);
    client->pKcp = NULL;
    client->conv = 0;
}

/**
 *
 * @param buf
 * @param len
 * @return
 */
static ssize_t kcpWriteData(char *buf, size_t len, void *context) {
    client_info_t *client = (client_info_t *) context;
    return ikcp_send(client->pKcp, buf, len);
}

/**
 *
 * @param totalLen
 * @param context
 * @return
 */
static ssize_t kcpForwardWriteFinish(size_t totalLen, void *context) {
    debugKcp("kcpForwardWriteFinish [%d] bytes", totalLen);
    // flush data, force to send
    client_info_t *client = (client_info_t *) context;
    ikcp_flush(client->pKcp);
    return totalLen;
}

/**
 *
 * @param buf
 * @param len
 * @return
 */
static ssize_t kcpReadData(char *buf, size_t len, void *context) {
    client_info_t *client = (client_info_t *) context;
    return ikcp_recv(client->pKcp, buf, len);
}

/**
 *  on read respond to onRead Event
 *
 *   it would be used by next layer
 *
 * @param buf
 * @param len
 */
static ssize_t kcpOnRead(char *buf, size_t len, void *context) {

    client_info_t *client = (client_info_t *) context;

    if (_kcpCtx.isServer) {

        IUINT32 conv = ikcp_getconv(buf);

        if (conv != client->conv) {
            // we need to create a new kcp conv
            kcpStopConv(context);
            if (NULL == kcpCreateConv(conv, context)) {
                errf("can not create new kcp conv for tunip [%s]", ipToString(client->tunIp));
                exit(-1);
            }
            logf("create new kcp conv [%lu] for tunip [%s]", conv, ipToString(client->tunIp));
        }
    }
    // set input
    return ikcp_input(client->pKcp, buf, len);
}

/**
 *
 * @param totalLen
 */
static ssize_t kcpFinishOnRead(size_t totalLen, void *context) {
    ssize_t readSize = 0;
    ssize_t totalSize = 0;

    while ((readSize = kcpReadData(_kcpCtx.dataBuffer, DATA_BUFFER_SIZE, context)) > 0) {
        totalSize += readSize;
        if (NULL != _kcpTransport.forwardRead) {
            debugKcp("kcp onRead [%d] bytes {%s}", readSize, log_hex_memory_32_bytes(_kcpCtx.dataBuffer));
            _kcpTransport.forwardRead(_kcpCtx.dataBuffer, readSize, context);
        }
    }

    if (totalSize > 0 && NULL != _kcpTransport.forwardReadFinish) {
        return _kcpTransport.forwardReadFinish(totalSize, context);
    }

    return totalLen;
}

/**
 *  set next layer transport
 *
 * @param transport
 */
static void kcpSetNextLayerTransport(struct itransport *transport) {
    transport->forwardRead = kcpOnRead;
    transport->forwardReadFinish = kcpFinishOnRead;
    // write to next layer
    _kcpTransport.forwardWrite = transport->writeData;
}

/**
 *
 * @param token
 * @param kcpConfig
 */
int sectunKcpParseConfig(jsmntok_t *token, const char *fileBuffer, sectun_kcp_config_t *kcpConfig) {

    if (JSMN_OBJECT != token->type || token->size <= 0) {
        errf("json kcp config bad format");
        return -1;
    }

    // read tokens
    int tokenSize = 2 * (token->size);
    int index;
    for (index = 1; index < tokenSize; index++) {
        jsmntok_t *tokenKey = &token[index];
        // a common string:value pair
        if (JSMN_STRING == tokenKey->type) {
            jsmntok_t *tokenValue = &token[index + 1];
            // skip value token
            index++;
            // get key:value pair
            const char *jsonKey = utilDupStr(fileBuffer + tokenKey->start, tokenKey->end - tokenKey->start);
            const char *jsonValue = utilDupStr(fileBuffer + tokenValue->start, tokenValue->end - tokenValue->start);

            if (0 == strcmp("nodelay", jsonKey)) {
                kcpConfig->nodelay = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("interval", jsonKey)) {
                kcpConfig->interval = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("resend", jsonKey)) {
                kcpConfig->resend = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("nc", jsonKey)) {
                kcpConfig->nc = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("sndwnd", jsonKey)) {
                kcpConfig->sndwnd = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("rcvwnd", jsonKey)) {
                kcpConfig->rcvwnd = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("mtu", jsonKey)) {
                kcpConfig->mtu = atoi(jsonValue);
                continue;
            }

            errf("unknow key [%s]", jsonKey);

        } else {
            errf("bad json kcp format config");
            return -1;
        }
    }

    return 0;
}

/**
 *
 * @param stream
 * @param kcpConfig
 */
void sectunKcpDumpConfig(FILE *stream, sectun_kcp_config_t *kcpConfig) {
    fprintf(stream, "nodelay : [%d]\n", kcpConfig->nodelay);
    fprintf(stream, "interval : [%d]\n", kcpConfig->interval);
    fprintf(stream, "resend : [%d]\n", kcpConfig->resend);
    fprintf(stream, "nc : [%d]\n", kcpConfig->nc);
    fprintf(stream, "sndwnd : [%d]\n", kcpConfig->sndwnd);
    fprintf(stream, "rcvwnd : [%d]\n", kcpConfig->rcvwnd);
    fprintf(stream, "mtu : [%d]\n", kcpConfig->mtu);
}

/**
 * 设置缺省参数
 * @param config
 */
void sectunKcpLoadDefaultConfig(sectun_kcp_config_t *config) {

    assert(NULL != config);

    memset(config, 0, sizeof(sectun_kcp_config_t));
    config->nodelay = 1;
    config->interval = 20;
    config->resend = 2;
    config->nc = 1;
    config->sndwnd = 1024;
    config->rcvwnd = 1024;
    config->mtu = 1380 + 24;
}

static int kcpStart() {
    // setup kcp update timer
    const sectun_event_t *pEvent = sectunGetEventInstance();
    pEvent->timer_init(&(_kcpCtx.timerEvent), timerKcpUpdate, NULL, _kcpCtx.config.interval, _kcpCtx.config.interval);
    pEvent->timer_start(&(_kcpCtx.timerEvent));
}

static int kcpStop() {
    // stop timer
    const sectun_event_t *pEvent = sectunGetEventInstance();
    pEvent->timer_stop(&(_kcpCtx.timerEvent));
}

/**
 *
 * @param client
 */
static void kcpCreateForClient(client_info_t *client) {
    kcpCreateConv(getClock(), client);
}

/**
 *  kcp 初始化参数设置
 *
 * @param config
 * @return
 */
int sectunKcpInit(sectun_kcp_config_t *config, int isServer) {

    assert(0 == _isKcpInit);

    // init kcp context
    memset(&_kcpCtx, 0, sizeof(_kcpCtx));

    // set whether is server
    _kcpCtx.isServer = isServer;

    // load the default config
    sectunKcpLoadDefaultConfig(&(_kcpCtx.config));

    if (NULL != config) {
        _kcpCtx.config = *config;
    }

    // client 初始创建一个 kcp 链接
    sectunAuthIterateClientArray(kcpCreateForClient);

    // setup _kcpTransport
    _kcpTransport = __dummyTransport;

    _kcpTransport.writeData = kcpWriteData;
    _kcpTransport.readData = kcpReadData;
    _kcpTransport.start = kcpStart;
    _kcpTransport.stop = kcpStop;
    _kcpTransport.forwardWriteFinish = kcpForwardWriteFinish;
    _kcpTransport.setNextLayer = kcpSetNextLayerTransport;

    // finish init
    _isKcpInit = 1;
    return 0;
}

/**
 *  singleton object
 *
 * @return
 */
struct itransport *const sectunGetKcpTransport() {
    // make sure kcp already init
    assert(_isKcpInit > 0);
    return &_kcpTransport;
}
