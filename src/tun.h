//
// Created by YuQiang on 2017-09-29.
//

#ifndef SECTUN_TUN_H
#define SECTUN_TUN_H

#include "itransport.h"

/**
 * init tun device
 *
 * @param dev  device name
 * @return
 */
int sectunTunInit(const char *dev);

/**
 *
 * 返回 singleton 的实例，常量不可更改
 *
 * */
struct itransport *const sectunGetTunTransport();

#endif //SECTUN_TUN_H
