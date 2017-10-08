//
// Created by YuQiang on 2017-09-30.
//

#ifndef SECTUN_UTIL_H
#define SECTUN_UTIL_H

#include <stdint.h>

/**
 *  dump string
 *
 * @param str
 * @param len
 * @return
 */
char *utilDupStr(const char *str, int len);

/**
 *  turn ip number to display string
 *
 * @param ip
 * @return
 */
const char *ipToString(uint32_t ip);

#endif //SECTUN_UTIL_H
