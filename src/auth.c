//
// Created by YuQiang on 2017-10-07.
//

#include <assert.h>

#include "inc.h"
#include "util.h"
#include "log.h"
#include "auth.h"

#ifdef DEBUG_AUTH
#define debugAuth(s...) __LOG(stdout, DEBUG_AUTH, s)
#else
#define debugAuth(s...)
#endif

/**
 * auth context
 */
static struct {
    client_info_t *tunIpToClientHash;
    client_info_t *tokenToClientHash;
} _authCtx;

static struct itransport _authTransport;
static int _isAuthInit = 0;

/**
 *  find client info by tunIp
 * @param tunIp
 * @return
 */
client_info_t *sectunAuthFindClientByTunIp(uint32_t tunIp) {
    client_info_t *client = NULL;
    HASH_FIND(tunIpToClient, _authCtx.tunIpToClientHash, &tunIp, sizeof(uint32_t), client);
    return client;
}

/**
 *  find client info by token
 * @param token
 * @return
 */
static client_info_t *authFindClientByToken(const char *token) {
    client_info_t *client = NULL;
    HASH_FIND(tokenToClient, _authCtx.tokenToClientHash, token, AUTH_USERTOKEN_LEN, client);
    return client;
}

/**
 *  add client into hash
 *
 * @param token
 * @param tunIp
 * @return
 */
int sectunAuthAddClient(const char *token, uint32_t tunIp) {
    // alloc client
    client_info_t *client = malloc(sizeof(client_info_t));
    memset(client, 0, sizeof(client_info_t));
    memcpy(client->userToken, token, AUTH_USERTOKEN_LEN);
    client->tunIp = tunIp;
    // add to hash
    HASH_ADD(tunIpToClient, _authCtx.tunIpToClientHash, tunIp, sizeof(uint32_t), client);
    HASH_ADD(tokenToClient, _authCtx.tokenToClientHash, userToken, AUTH_USERTOKEN_LEN, client);
    return 0;
}

/**
 *  write data
 *
 *  ---------------------------------
 *     data  |  user token | tun ip
 *  ---------------------------------
 *
 * @param buffer
 * @param len
 * @param context
 * @return
 */
static ssize_t authWriteData(char *buffer, size_t len, void *context) {

    if (len + AUTH_USERTOKEN_LEN + AUTH_TUNIP_LEN >= DATA_BUFFER_SIZE) {
        errf("authWriteData Error len [%d] + AUTH_USERTOKEN_LEN [%d] + AUTH_TUNIP_LEN [%d] >= DATA_BUFFER_SIZE [%d]",
             len, AUTH_USERTOKEN_LEN, AUTH_TUNIP_LEN, DATA_BUFFER_SIZE);
        return -1;
    }

    client_info_t *client = (client_info_t *) context;

    // put user token at the end of the package
    debugAuth("authWriteData user token[%.*s]", AUTH_USERTOKEN_LEN, client->userToken);
    // copy user token
    memcpy(buffer + len, client->userToken, AUTH_USERTOKEN_LEN);
    // copy tun ip
    memcpy(buffer + len + AUTH_USERTOKEN_LEN, &(client->tunIp), AUTH_TUNIP_LEN);

    len += AUTH_USERTOKEN_LEN + AUTH_TUNIP_LEN;

    if (NULL != _authTransport.forwardWrite) {
        return _authTransport.forwardWrite(buffer, len, context);
    }

    return len;
}

/**
 * forward read
 *
 * @param buffer
 * @param len
 * @param context
 * @return
 */
static ssize_t authForwardRead(char *buffer, size_t len, void *context) {

    // verify whether packet contains valid user token
    uint32_t tunIp = (uint32_t) *(buffer + len - AUTH_TUNIP_LEN);
    const char *token = buffer + len - AUTH_TUNIP_LEN - AUTH_USERTOKEN_LEN;

    client_info_t *client = sectunAuthFindClientByTunIp(tunIp);
    if (NULL == client || 0 != strncmp(client->userToken, token, AUTH_USERTOKEN_LEN)) {
        errf("invalid user token [%.*s] tunIp [%s]", AUTH_USERTOKEN_LEN, token, ipToString(tunIp));
        return -1;
    }

    // remove user token tun ip from packet
    len -= (AUTH_USERTOKEN_LEN + AUTH_TUNIP_LEN);

    // update client peerAddr
    client_info_t *tmpClient = (client_info_t *) context;

    // check new connect
    if (client->peerAddr.sin_port != tmpClient->peerAddr.sin_port) {
        logf("auth user token [%.*s] tunip [%s] connect from [%s:%lu]",
             AUTH_USERTOKEN_LEN, client->userToken, ipToString(client->tunIp),
             inet_ntoa(((struct sockaddr_in *) &(tmpClient->peerAddr))->sin_addr),
             ntohs(tmpClient->peerAddr.sin_port));
    }

    client->peerAddr = tmpClient->peerAddr;
    client->peerAddrLen = tmpClient->peerAddrLen;
    // copy user token to tmp client
    memcpy(tmpClient->userToken, client->userToken, AUTH_USERTOKEN_LEN);

    debugAuth("authForwardRead user token[%.*s]", AUTH_USERTOKEN_LEN, client->userToken);

    if (_authTransport.forwardRead) {
        return _authTransport.forwardRead(buffer, len, client);
    }

    return len;
}

/**
 *
 * @param totalLen
 * @param context
 * @return
 */
static ssize_t authForwardReadFinish(size_t totalLen, void *context) {
    if (NULL != _authTransport.forwardReadFinish) {
        client_info_t *tmpClient = (client_info_t *) context;
        client_info_t *client = authFindClientByToken(tmpClient->userToken);
        if (NULL == client) {
            errf("invalid user token [%.*s] recv", AUTH_USERTOKEN_LEN, tmpClient->userToken);
            return -1;
        }
        debugAuth("authForwardReadFinish user token[%.*s]", AUTH_USERTOKEN_LEN, client->userToken);
        return _authTransport.forwardReadFinish(totalLen, client);
    }
    return totalLen;
}

/**
 *
 * @param transport
 */
static void authSetNextLayer(struct itransport *transport) {
    transport->forwardRead = authForwardRead;
    transport->forwardReadFinish = authForwardReadFinish;
    _authTransport.forwardWrite = transport->writeData;
}

/**
 *
 * @param tokenStr
 * @param tunIp
 * @param isServer
 * @return
 */
int sectunAuthInit(const char *tokenStr, uint32_t tunIp, int isServer) {
    memset(&_authCtx, 0, sizeof(_authCtx));

    assert(0 == _isAuthInit);

    int ret = 0;

    if (!isServer) {
        // client add it self
        ret = sectunAuthAddClient(tokenStr, tunIp);
        if (0 != ret) {
            return ret;
        }
    } else {
        // server, need to add a list of client
        char *tokenList = utilDupStr(tokenStr, strlen(tokenStr));

        char *token = strtok(tokenList, AUTH_USERTOKEN_DELIMITER);
        while (NULL != token) {
            tunIp++;
            ret = sectunAuthAddClient(token, tunIp);
            if (0 != ret) {
                return ret;
            }
            token = strtok(NULL, AUTH_USERTOKEN_DELIMITER);
        }
    }

    // init auth transport
    _authTransport = __dummyTransport;
    _authTransport.writeData = authWriteData;
    _authTransport.setNextLayer = authSetNextLayer;

    // finish init
    _isAuthInit = 1;
    return 0;
}

/**
 *
 * @return
 */
struct itransport *const sectunGetAuthTransport() {
    assert(_isAuthInit > 0);
    return &_authTransport;
}

/**
 * iterate all client info , call func that user provide
 *
 * @param func
 */
void sectunAuthIterateClientArray(void(*func)(client_info_t *client)) {
    if (NULL == func) {
        return;
    }

    client_info_t *client, *tmp;
    HASH_ITER(tunIpToClient, _authCtx.tunIpToClientHash, client, tmp) {
        func(client);
    }
}

/**
 *  dump client info for debug purpose
 *
 * @param stream
 */
void sectunAuthDumpClient(FILE *stream) {
    client_info_t *client, *tmp;
    HASH_ITER(tunIpToClient, _authCtx.tunIpToClientHash, client, tmp) {
        fprintf(stream, "userToken [%.*s] assign netip : [%s]\n", AUTH_USERTOKEN_LEN,
                client->userToken, ipToString(client->tunIp));
    }
}