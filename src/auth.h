//
// Created by YuQiang on 2017-10-07.
//

#ifndef SECTUN_GITHUB_AUTH_H
#define SECTUN_GITHUB_AUTH_H

#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "3rd/uthash/uthash.h"

/* the structure to store known client addresses for the server */
typedef struct {
    // user token
    char userToken[AUTH_USERTOKEN_LEN];

    // source address of UDP
    struct sockaddr_storage sourceAddr;
    socklen_t sourceAddrLen;

    // input tun IP
    // in network order
    uint32_t tunIp;

    UT_hash_handle hh;
} client_info_t;


client_info_t *sectunAuthFindClientByTunIp(uint32_t tunIp);

int sectunAuthAddClient(const char *token, uint32_t tunIp);

int sectunAuthInit();

int sectunAuthStop();


#endif //SECTUN_GITHUB_AUTH_H
