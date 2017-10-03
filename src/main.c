
/*
 * Created by YuQiang on 2017-09-30.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "inc.h"
#include "util.h"
#include "args.h"
#include "shell.h"
#include "event.h"
#include "log.h"
#include "udp.h"
#include "encrypt.h"
#include "heartbeat.h"
#include "kcp.h"
#include "tun.h"

#ifdef DEBUG_MAIN
#define debugMain(s...) __LOG(stdout, DEBUG_MAIN, s)
#else
#define debugMain(s...)
#endif

/**
 * argument
 */
static sectun_args_t _sectunArgs;

/**
 * transport chain
 */
static struct itransport *_transChain[TRANSPORT_CHAIN_MAX];
static int _transChainIndexMax = 0;

/**
 *  restart all other transports behind transport
 *
 * @param transport
 * @return
 */
int sectunRestartTransport(const struct itransport *transport) {

    int pos = 0;
    // find position of transport
    for (pos = 0; pos < _transChainIndexMax; pos++) {
        if (transport == _transChain[pos]) {
            break;
        }
    }
    // not found, do nothing
    if (pos >= _transChainIndexMax) {
        errf("sectunRestartTransport find no transport to begin");
        return 0;
    }
    logf("restart transport from pos [%d]", pos);

    int index = pos;
    int ret = 0;

    // stop all transport
    for (index = pos; index < _transChainIndexMax; index++) {
        if ((ret = _transChain[index]->stop()) < 0) {
            errf("stop transport [%d] error ret [%d]", index, ret);
            return ret;
        }
        logf("stop transport [%d] success", index);
    }

    //start all transport
    for (index = _transChainIndexMax - 1; index >= pos; index--) {
        if ((ret = _transChain[index]->start()) < 0) {
            errf("start transport [%d] error ret [%d]", index, ret);
            return ret;
        }
        logf("start transport [%d] success", index);
    }

    logf("restart transport finish");
    return 0;
}

/**
 * start the chain
 *
 * @return
 */
static int startTransChain() {
    // chain all transport
    int ret = 0;
    int tranIndex = 0;
    for (tranIndex = _transChainIndexMax - 1; tranIndex >= 0; tranIndex--) {
        ret = _transChain[tranIndex]->start();
        if (ret < 0) {
            errf("_transChain[%d] start fail with ret [%d]", tranIndex, ret);
            return ret;
        }
        debugMain("_transChain[%d] start with ret [%d]", tranIndex, ret);
    }
    return ret;
}

/**
 * stop the train
 *
 * @return
 */
static int stopTransChain() {
    // chain all transport
    int ret = 0;
    int tranIndex = 0;
    for (tranIndex = 0; tranIndex < _transChainIndexMax; tranIndex++) {
        ret = _transChain[tranIndex]->stop();
        if (ret < 0) {
            return ret;
        }
    }
    return ret;
}

static int setupTransChain(sectun_args_t *args) {

    int isServer = (SECTUN_MODE_SERVER == args->mode);

    int ret = 0;

    // init data
    memset(_transChain, 0, TRANSPORT_CHAIN_MAX);

    // setup tun device
    if (0 != (ret = sectunTunInit(args->device))) {
        return ret;
    }
    // set tun transport
    logf("tun device ready");
    _transChain[_transChainIndexMax++] = sectunGetTunTransport();

    /**************************************************************************
     *
     *  transport chain such as
     *
     *  kcp+heartbeat+encrypt+udp
     *  heartbeat+encrypt+udp
     *  encrypt+udp
     *  udp
     *
     * ************************************************************************/

    char *transChainStr = utilDupStr(args->transport, strlen(args->transport));
    char *transStr = strtok(transChainStr, TRANSPORT_DELIMITER);

    while (NULL != transStr && _transChainIndexMax < TRANSPORT_CHAIN_MAX) {

        if (0 == strcmp("kcp", transStr)) {
            // setup kcp transport
            if (0 != (ret = sectunKcpInit(&(args->kcpConfig), isServer))) {
                return ret;
            }
            logf("kcp transport ready");
            _transChain[_transChainIndexMax++] = sectunGetKcpTransport();
        }

        if (0 == strcmp("heartbeat", transStr)) {
            // setup heartbeat transport
            // interval 30 seconds, timeout 60 seconds
            if (0 != (ret = sectunHeartbeatInit(args->heartbeatInterval,
                                                args->heartbeatTimeout, isServer))) {
                return ret;
            }
            logf("heartbeat transport ready");
            _transChain[_transChainIndexMax++] = sectunGetHearbeatTransport();
        }

        if (0 == strcmp("encrypt", transStr)) {
            // setup encrypt transport
            if (0 != (ret = sectunEncryptInit(args->encrypt, args->encryptKey))) {
                return ret;
            }
            logf("encrypt transport ready");
            _transChain[_transChainIndexMax++] = sectunGetEncryptTransport();
        }

        if (0 == strcmp("udp", transStr)) {
            // setup udp transport
            if (0 != (ret = sectunUdpInit(args->host, args->port, isServer))) {
                return ret;
            }
            logf("udp transport ready");
            _transChain[_transChainIndexMax++] = sectunGetUdpTransport();
        }

        // next transport string
        transStr = strtok(NULL, TRANSPORT_DELIMITER);
    }


    // chain all transport
    int tranIndex = 0;
    for (tranIndex = 0; tranIndex < _transChainIndexMax - 1; tranIndex++) {
        _transChain[tranIndex]->setNextLayer(_transChain[tranIndex + 1]);
    }

    return ret;
}


/**
 *  stop sectun
 *
 * @param args
 */
static int sectunStop(sectun_args_t *args) {

    // run down script
    sectunShellRun(args->downScript);

    // stop transport chain
    stopTransChain();

    const sectun_event_t *const pEvent = sectunGetEventInstance();
    return pEvent->exit();
}

/**
 *  signal handle
 *
 * @param signo
 */
static void sig_handler(int signo) {
    if (signo == SIGINT) {
        sectunStop(&_sectunArgs);
        exit(1);  // for gprof
    } else {
        sectunStop(&_sectunArgs);
    }
}

/**
 *  start sectun
 *
 * @param args
 * @return
 */
static int sectunStart(sectun_args_t *args) {

    const sectun_event_t *const pEvent = sectunGetEventInstance();

    int ret = 0;

    if ((ret = setupTransChain(args)) != 0) {
        errf("setup transport fail [%d]", ret);
        return ret;
    }

    // start the chain
    if ((ret = startTransChain()) != 0) {
        errf("start transport fail [%d]", ret);
        return ret;
    }

    // call upScript
    ret = sectunShellRun(args->upScript);

    if (SECTUN_MODE_SERVER == args->mode) {
        logf("server is ready");
    } else {
        logf("client is ready");
    }

    return pEvent->run(0);
}


int main(int argc, char *argv[]) {

    // parse args
    sectunArgParse(&_sectunArgs, argc, argv);
    sectunArgDump(stdout, &_sectunArgs);

    // setup signal handle
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // start sectun
    return sectunStart(&_sectunArgs);
}
