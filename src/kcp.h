//
// Created by YuQiang on 2017-09-21.
//

#ifndef SECTUN_KCP_H
#define SECTUN_KCP_H

#include "3rd/jsmn/jsmn.h"

#include "itransport.h"


/**
 * kcp 配置选项
 */
typedef struct {

    int nodelay;
    int interval;
    int resend;
    int nc;
    int sndwnd;
    int rcvwnd;
    int mtu;

} sectun_kcp_config_t;


/**
 *  parse kcp config from json file
 * @param token
 * @param kcpConfig
 */
int sectunKcpParseConfig(jsmntok_t *token, const char *fileBuffer, sectun_kcp_config_t *kcpConfig);

/**
 *
 * @param stream
 * @param kcpConfig
 */
void sectunKcpDumpConfig(FILE *stream, sectun_kcp_config_t *kcpConfig);

/**
 * 设置缺省的配置
 *
 * @param config
 */
void sectunKcpLoadDefaultConfig(sectun_kcp_config_t *config);

/**
 * 配置 kcp 参数
 *
 * @param config
 * @return
 */
int sectunKcpInit(sectun_kcp_config_t *config, int isServer);

/**
 * kcp 的 singleton 实现
 *
 * @return
 */
struct itransport *const sectunGetKcpTransport();

#endif //SECTUN_KCP_H
