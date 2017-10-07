//
// Created by YuQiang on 2017-10-07.
//

#ifndef SECTUN_GITHUB_AUTH_H
#define SECTUN_GITHUB_AUTH_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "3rd/uthash/uthash.h"
#include "3rd/ikcp/ikcp.h"

#include "itransport.h"

#define AUTH_USERTOKEN_LEN  8
#define AUTH_USERTOKEN_DELIMITER    ","

/* the structure to store known client addresses for the server */
typedef struct {
    // user token
    char userToken[AUTH_USERTOKEN_LEN];

    // source address of UDP
    struct sockaddr_in peerAddr;
    socklen_t peerAddrLen;

    // input tun IP
    // in network order
    uint32_t tunIp;

    // kcp connection number
    IUINT32 conv;
    // kcp object
    ikcpcb *pKcp;

    UT_hash_handle tunIpToClient;
    UT_hash_handle tokenToClient;
} client_info_t;


client_info_t *sectunAuthFindClientByTunIp(uint32_t tunIp);

client_info_t *sectunAuthFindClientByToken(const char *token);

int sectunAuthAddClient(const char *token, uint32_t tunIp);

int sectunAuthInit(const char *tokenStr, uint32_t tunIp, int isServer);

int sectunAuthStop();

void sectunAuthDumpClient(FILE *stream);

/**
 *
 * 返回 singleton 的实例，常量不可更改
 *
 * */
struct itransport *const sectunGetAuthTransport();


#endif //SECTUN_GITHUB_AUTH_H
