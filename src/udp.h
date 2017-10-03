//
// Created by YuQiang on 2017-09-17.
//

#ifndef SECTUN_UDP_H
#define SECTUN_UDP_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "itransport.h"

/**
 * init udp socket
 *
 * @param host  host ip to connect or bind
 * @param port  connect to port or listen
 * @param isServer  whether it is a client or server, 0 client, 1 server
 *
 * */
int sectunUdpInit(const char *host, int port, int isServer);

/**
 *
 * 返回 singleton 的实例，常量不可更改
 *
 * */
struct itransport *const sectunGetUdpTransport();

#endif //SECTUN_UDP_H
