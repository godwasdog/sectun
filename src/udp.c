//
// Created by YuQiang on 2017-09-17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>

#include "inc.h"
#include "log.h"
#include "event.h"
#include "util.h"
#include "auth.h"
#include "udp.h"

#ifdef DEBUG_UDP
#define debugUdp(s...) __LOG(stdout, DEBUG_UDP, s)
#else
#define debugUdp(s...)
#endif

// udp context
static struct {

    int socketFd;

    // tmp client info, save socket addr info
    client_info_t tmpClientInfo;

    // save host/port
    const char *host;
    int port;

    // whether we are server
    int isServer;
    // read/write buffer
    char dataBuffer[DATA_BUFFER_SIZE];

    // event handle to respond all event
    uev_t udpEvent;

} _udpCtx;

// transport interface
static struct itransport _udpTransport;
static int _isUdpInit = 0;


/**
 *
 * @param buf
 * @param len
 * @return
 */
static ssize_t udpWriteData(char *buf, size_t len, void *context) {
    debugUdp("udp write data --[%d] bytes {%s}", len, log_hex_memory_32_bytes(buf));
    client_info_t *client = (client_info_t *) context;


#ifdef DEBUG_UDP

    struct in_addr in;
    in.s_addr = htonl((uint32_t) client->tunIp);
    debugUdp("udpWriteData  tunip : [%s] \n", inet_ntoa(in));

#endif

    return sendto(_udpCtx.socketFd, buf, len, 0,
                  (const struct sockaddr *) &(client->peerAddr), client->peerAddrLen);
}

/**
 *
 * @param buf
 * @param len
 * @param fromAddr
 * @return
 */
static ssize_t udpReadDataFrom(char *buf, size_t len, struct sockaddr *fromAddr, socklen_t *addrlen) {
    return recvfrom(_udpCtx.socketFd, buf, len, 0, fromAddr, addrlen);
}

/**
 *
 * @param buf
 * @param len
 * @return
 */
static ssize_t udpReadData(char *buf, size_t len, void *context) {
    client_info_t *client = (client_info_t *) context;
    return udpReadDataFrom(buf, len, (struct sockaddr *) &(client->peerAddr), &(client->peerAddrLen));
}

/**
 * dummy function for ReadEvent
 * we just read and discard all data
 *
 * @param w
 * @param arg
 * @param events
 */
static void udpOnRead(uev_t *w, void *arg, int events) {
    ssize_t readSize = 0;
    ssize_t totalSize = 0;
    void *context = NULL;
    while ((readSize = udpReadData(_udpCtx.dataBuffer, DATA_BUFFER_SIZE, &(_udpCtx.tmpClientInfo))) > 0) {
        debugUdp("udp read data --[%d] bytes {%s}", readSize, log_hex_memory_32_bytes(_udpCtx.dataBuffer));
        totalSize += readSize;
        if (NULL != _udpTransport.forwardRead) {
            _udpTransport.forwardRead(_udpCtx.dataBuffer, readSize, &(_udpCtx.tmpClientInfo));
        }
    }

    if (totalSize > 0 && NULL != _udpTransport.forwardReadFinish) {
        _udpTransport.forwardReadFinish(totalSize, &(_udpCtx.tmpClientInfo));
    }
}

static void udpInitClientPeer(client_info_t *client) {
    client->peerAddr = _udpCtx.tmpClientInfo.peerAddr;
    client->peerAddrLen = _udpCtx.tmpClientInfo.peerAddrLen;
}

/**
 *  create a socket and start the event
 * @return
 */
static int udpStart() {

    assert(0 == _udpCtx.socketFd);

    struct hostent *hostent;
    hostent = gethostbyname(_udpCtx.host);
    if (NULL == hostent) {
        errf("can not resolve host %s", _udpCtx.host);
        return -1;
    }

    /* initialize socket */
    if ((_udpCtx.socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        errf("can not create socket");
        return -1;
    }

    // set non-block
    int socketFlag = -1;
    if ((socketFlag = fcntl(_udpCtx.socketFd, F_GETFL, 0)) < 0
        || fcntl(_udpCtx.socketFd, F_SETFL, socketFlag | O_NONBLOCK) < 0) {
        errf("can not set socket to nonblock");
        return -1;
    }

    memset(&(_udpCtx.tmpClientInfo.peerAddr), 0, sizeof(_udpCtx.tmpClientInfo.peerAddr));
    _udpCtx.tmpClientInfo.peerAddrLen = sizeof(_udpCtx.tmpClientInfo.peerAddr);
    _udpCtx.tmpClientInfo.peerAddr.sin_family = AF_INET;
    _udpCtx.tmpClientInfo.peerAddr.sin_port = htons(_udpCtx.port);
    _udpCtx.tmpClientInfo.peerAddr.sin_addr = *((struct in_addr *) hostent->h_addr);

    if (_udpCtx.isServer &&
        bind(_udpCtx.socketFd, (struct sockaddr *) &(_udpCtx.tmpClientInfo.peerAddr),
             _udpCtx.tmpClientInfo.peerAddrLen) == -1) {
        errf("server can not bind %s:%d", _udpCtx.host, _udpCtx.port);
        return -1;
    }

    // 把所有 client 的 peer 初始化
    sectunAuthIterateClientArray(udpInitClientPeer);

    // setup event handle
    const sectun_event_t *const pEvent = sectunGetEventInstance();
    pEvent->io_init(&(_udpCtx.udpEvent), udpOnRead, NULL, _udpCtx.socketFd, UEV_READ);
    pEvent->io_start(&(_udpCtx.udpEvent));

    return 0;
}

/**
 * stop event and close a socket
 */
static int udpStop() {
    assert(_udpCtx.socketFd > 0);

    // stop event handle
    const sectun_event_t *const pEvent = sectunGetEventInstance();
    pEvent->io_stop(&(_udpCtx.udpEvent));

    // close socket
    close(_udpCtx.socketFd);
    _udpCtx.socketFd = 0;

    return 0;
}

/**
 * init socket
 *
 * @param host  host ip to connect or bind
 * @param port  connect to port or listen
 * @param isServer  whether it is a client or server, 0 client, 1 server
 *
 * */
int sectunUdpInit(const char *host, int port, int isServer) {

    assert(0 == _isUdpInit);

    // init udp context
    memset(&_udpCtx, 0, sizeof(_udpCtx));
    _udpCtx.host = host;
    _udpCtx.port = port;
    _udpCtx.isServer = isServer;


    // setup transport interface
    _udpTransport = __dummyTransport;
    _udpTransport.writeData = udpWriteData;
    _udpTransport.readData = udpReadData;
    _udpTransport.start = udpStart;
    _udpTransport.stop = udpStop;

    // finish init
    _isUdpInit = 1;
    return 0;
}

/**
 *
 * @return
 */
struct itransport *const sectunGetUdpTransport() {
    assert(_isUdpInit > 0);
    return &_udpTransport;
}
