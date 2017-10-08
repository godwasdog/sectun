//
// Created by YuQiang on 2017-10-03.
//

#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "inc.h"
#include "event.h"
#include "itransport.h"
#include "log.h"
#include "auth.h"
#include "heartbeat.h"

#ifdef DEBUG_HEARTBEAT
#define debugHeartbeat(s...) __LOG(stdout, DEBUG_HEARTBEAT, s)
#else
#define debugHeartbeat(s...)
#endif

#define HEARTBEAT_MAGIC_NOP   0xAE
#define HEARTBEAT_MAGIC_DATA  0xEA

static struct itransport _hearbeatTransport;
static int _isHeartbeatInit = 0;

static struct {
    int isServer;
    unsigned long timestamp;
    // timer used to send msg
    int interval;
    int timeout;
    uev_t timerEvent;
} _hearbeatCtx;


/**
 *  get heartbeat timestamp in seconds
 * @return
 */
static unsigned long getTimestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return (unsigned long) te.tv_sec;
}

static ssize_t sendHeartbeatMsg(void *context) {
    static char buffer[64];
    memset(buffer, '\0', 64);
    // we send timestamp here only use it as random number
    sprintf(buffer, "%lu", getTimestamp());
    int len = strlen(buffer);
    buffer[len++] = HEARTBEAT_MAGIC_NOP;

    debugHeartbeat("send hearbeat msg");

    return _hearbeatTransport.forwardWrite(buffer, len, context);
}

static void sendHeartbeatMsg_1(client_info_t *client) {
    sendHeartbeatMsg(client);
}

/**
 *  only client would run this timer, server will never run this
 *
 * @param w
 * @param arg
 * @param events
 */
extern int sectunRestartTransport(const struct itransport *transport);

void timerHeartbeat(uev_t *w, void *arg, int events) {

    unsigned long currentTimestamp = getTimestamp();
    if (currentTimestamp - _hearbeatCtx.timestamp > _hearbeatCtx.timeout) {
        // timeout , we need to restart all transport behind heartbeat
        errf("heartbeat timeout, restart transport");
        sectunRestartTransport(&_hearbeatTransport);
        return;
    }
    //send hearbeat
    sectunAuthIterateClientArray(sendHeartbeatMsg_1);
}

/**
 *
 * @param buffer
 * @param len
 * @return
 */
static ssize_t hearbeatWriteData(char *buffer, size_t len, void *context) {

    const maxWriteSize = DATA_BUFFER_SIZE - DATA_PADDING_SIZE - 1;
    if (len >= maxWriteSize) {
        errf("data is too long len [%d] maxSize [%d]", len, maxWriteSize);
        return -1;
    }

    buffer[len++] = HEARTBEAT_MAGIC_DATA;
    return _hearbeatTransport.forwardWrite(buffer, len, context);
}


ssize_t hearbeatForwardRead(char *buffer, size_t len, void *context) {

    unsigned char magic = buffer[--len];

    if (HEARTBEAT_MAGIC_NOP == magic) {

        debugHeartbeat("recv hearbeat msg");

        if (_hearbeatCtx.isServer) {
            // server send back msg
            sendHeartbeatMsg(context);
        } else {
            //client update timestamp
            _hearbeatCtx.timestamp = getTimestamp();
            debugHeartbeat("hearbeat update timestamp [%lu]", _hearbeatCtx.timestamp);
        }
        return len;
    }

    if (HEARTBEAT_MAGIC_DATA == magic) {
        //forward data
        return _hearbeatTransport.forwardRead(buffer, len, context);
    }

    // invalid packet, we may under attack, drop the packet here
    debugHeartbeat("heartbeat bad magic char [%02x]", magic);
    return len;
}

ssize_t heartbeatForwardReadFinish(size_t totalLen, void *context) {
    return _hearbeatTransport.forwardReadFinish(totalLen, context);
}


static int heartbeatStart() {

    if (_hearbeatCtx.isServer) {
        return 0;
    }
    // only client would send hearbeat msg
    _hearbeatCtx.timestamp = getTimestamp();

    // setup timer
    const sectun_event_t *pEvent = sectunGetEventInstance();
    pEvent->timer_init(&(_hearbeatCtx.timerEvent), timerHeartbeat,
                       NULL, _hearbeatCtx.interval, _hearbeatCtx.interval);
    pEvent->timer_start(&(_hearbeatCtx.timerEvent));

    logf("start heartbeat interval [%d]sec timeout [%d]sec", _hearbeatCtx.interval / 1000, _hearbeatCtx.timeout);

    return 0;
}

static int heartbeatStop() {

    if (_hearbeatCtx.isServer) {
        return 0;
    }
    // only client would send hearbeat msg
    const sectun_event_t *pEvent = sectunGetEventInstance();
    pEvent->timer_stop(&(_hearbeatCtx.timerEvent));

    logf("stop hearbeat");

    return 0;
}

/**
 *
 * @param transport
 */
static void heartbeatSetNextLayer(struct itransport *transport) {
    _hearbeatTransport.forwardWrite = transport->writeData;

    transport->forwardRead = hearbeatForwardRead;
    transport->forwardReadFinish = heartbeatForwardReadFinish;
}

/**
 *
 * @param interval
 * @param timeout
 * @param isServer
 * @return
 */
int sectunHeartbeatInit(int interval, int timeout, int isServer) {

    assert(0 == _isHeartbeatInit);

    // init context
    memset(&_hearbeatCtx, 0, sizeof(_hearbeatCtx));
    _hearbeatCtx.isServer = isServer;
    _hearbeatCtx.interval = 1000 * interval;
    _hearbeatCtx.timeout = timeout;


    // init transport
    _hearbeatTransport = __dummyTransport;

    _hearbeatTransport.writeData = hearbeatWriteData;
    _hearbeatTransport.start = heartbeatStart;
    _hearbeatTransport.stop = heartbeatStop;
    _hearbeatTransport.setNextLayer = heartbeatSetNextLayer;

    _isHeartbeatInit = 1;
    return 0;
}

/**
 *
 * @return
 */
struct itransport *const sectunGetHearbeatTransport() {
    assert(_isHeartbeatInit > 0);
    return &_hearbeatTransport;
}
