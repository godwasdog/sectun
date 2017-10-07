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
} _authCtx;

/**
 *  find client info by tunIp
 * @param tunIp
 * @return
 */
client_info_t *sectunAuthFindClientByTunIp(uint32_t tunIp) {
    client_info_t *client;
    HASH_FIND_INT(_authCtx.tunIpToClientHash, tunIp, client);
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
    client_info_t *client = malloc(sizeof(client_info_t));
    memcpy(client->userToken, token, AUTH_USERTOKEN_LEN);
    client->tunIp = tunIp;
    HASH_ADD_INT(_authCtx.tunIpToClientHash, tunIp, client);
    return 0;
}


int sectunAuthInit() {
    memset(_authCtx, 0, sizeof(_authCtx));
    return 0;
}

int sectunAuthStop() {
    client_info_t *client, *tmp;
    HASH_ITER(hh, _authCtx.tunIpToClientHash, client, tmp) {
        HASH_DEL(_authCtx.tunIpToClientHash, client);
        free(client);
    }
    return 0;
}
