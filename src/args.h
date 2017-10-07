/**
 * args.h
*/

#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "kcp.h"

typedef enum {
    SECTUN_MODE_NONE = 0,
    SECTUN_MODE_SERVER,
    SECTUN_MODE_CLIENT
} sectun_mode;

typedef enum {
    SECTUN_CMD_NONE = 0,
    SECTUN_CMD_START,
    SECTUN_CMD_STOP,
    SECTUN_CMD_RESTART
} sectun_cmd;

typedef struct {
    sectun_mode mode;
    sectun_cmd cmd;

    const char *confFile;

    const char *device;
    const char *host;
    uint16_t port;
    uint16_t mtu;
    const char *encryptKey;
    const char *encrypt;
    unsigned int heartbeatInterval;
    unsigned int heartbeatTimeout;
    const char *transport;
    // the ip of the "net" configuration
    // in host order
    uint32_t netip;
    // token used by client, 8 character size, such as "8wegno3a"
    const char *userToken;
    // token list used by server, seprated by comma, such as  "8wegno3a,8webno3a,ewegno3a"
    const char *userTokenList;

    const char *upScript;
    const char *downScript;

    // config for each module
    sectun_kcp_config_t kcpConfig;

} sectun_args_t;

/**
 *  parse command line and all configurations
 *
 * @param args
 * @param argc
 * @param argv
 * @return
 */
int sectunArgParse(sectun_args_t *args, int argc, char **argv);

/**
 * dump args value
 *
 * @param stream
 * @param args
 */
void sectunArgDump(FILE *stream, sectun_args_t *args);

#endif
