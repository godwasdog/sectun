//
// Created by YuQiang on 2017-10-07.
//


#include "inc.h"
#include "auth.h"

/**
 * auth context
 */
static struct {
    client_info_t *tunIpToClientHash;
    client_info_t *tokenToClientHash;
} _authCtx;

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
client_info_t *sectunAuthFindClientByToken(const char *token) {
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
    memcpy(client->userToken, token, AUTH_USERTOKEN_LEN);
    client->tunIp = tunIp;
    // add to hash
    HASH_ADD(tunIpToClient, _authCtx.tunIpToClientHash, tunIp, sizeof(uint32_t), client);
    HASH_ADD(tokenToClient, _authCtx.tokenToClientHash, userToken, AUTH_USERTOKEN_LEN, client);
    return 0;
}


int sectunAuthInit() {
    memset(&_authCtx, 0, sizeof(_authCtx));
    return 0;
}

int sectunAuthStop() {
    return 0;
}
