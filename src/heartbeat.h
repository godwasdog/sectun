//
// Created by YuQiang on 2017-10-03.
//

#ifndef SECTUN_HEARTBEAT_H
#define SECTUN_HEARTBEAT_H


/**
 *
 * @param interval   多长时间发送过一次心跳 senconds
 * @param timeout    多长时间算超时  seconds
 * @return
 */
int sectunHeartbeatInit(int interval, int timeout, int isServer);


/**
 *  singleton
 *
 * @return
 */
struct itransport *const sectunGetHearbeatTransport();

#endif //SECTUN_HEARTBEAT_H
