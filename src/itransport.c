//
// Created by YuQiang on 2017-10-01.
//

#include <assert.h>

#include "inc.h"
#include "log.h"
#include "itransport.h"

#ifdef DEBUG_TRANSPORT
#define debugTransport(s...) __LOG(stdout, DEBUG_TRANSPORT, s)
#else
#define debugTransport(s...)
#endif

static ssize_t dummyReadData(char *buffer, size_t len, void *context) {
    debugTransport("__dummyTransport.readData [%d] bytes", len);
    return len;
}


static ssize_t dummyWriteData(char *buffer, size_t len, void *context) {
    debugTransport("__dummyTransport.writeData [%d] bytes", len);
    return len;
}

static int dummyStart() {
    debugTransport("__dummyTransport.start");
    return 0;
}

static int dummyStop() {
    debugTransport("__dummyTransport.stop");
    return 0;
}

static void dumySetNextLayer(struct itransport *transport) {
    assert(NULL != transport);
    debugTransport("__dummyTransport.setNextLayer");
}

static ssize_t dummyForwardRead(char *buffer, size_t len, void *context) {
    debugTransport("__dummyTransport.forwardRead [%d] bytes", len);
    return len;
}

static ssize_t dummyForwardReadFinish(size_t totalLen, void *context) {
    debugTransport("__dummyTransport.forwardReadFinish [%d] bytes", totalLen);
    return totalLen;
}

static ssize_t dummyForwardWrite(char *buffer, size_t len, void *context) {
    debugTransport("__dummyTransport.forwardWrite [%d] bytes", len);
    return len;
}

static ssize_t dummyForwardWriteFinish(size_t totalLen, void *context) {
    debugTransport("__dummyTransport.forwardWriteFinish [%d] bytes", totalLen);
    return totalLen;
}

// dummy transport for use
struct itransport __dummyTransport = {
        .readData = dummyReadData,
        .writeData = dummyWriteData,
        .start = dummyStart,
        .stop = dummyStop,
        .setNextLayer = dumySetNextLayer,
        .forwardRead = dummyForwardRead,
        .forwardReadFinish = dummyForwardReadFinish,
        .forwardWrite = dummyForwardWrite,
        .forwardWriteFinish = dummyForwardWriteFinish
};
