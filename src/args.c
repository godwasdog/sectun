/**
 *
 * args.c
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "3rd/jsmn/jsmn.h"

#include "inc.h"
#include "args.h"
#include "log.h"
#include "kcp.h"
#include "util.h"

static const char *_helpMessage =
        "usage: sectun -c config_file -s start/stop/restart [-v]\n"
                "\n"
                "  -h, --help            show this help message and exit\n"
                "  -s start/stop/restart control sectun process. if omitted, will run\n"
                "                        in foreground\n"
                "  -c config_file        path to config file\n"
                "  -v                    verbose logging\n"
                "\n"
                "Secrect Tunnel developed for geek guys version [%s]\n"
                "Online help: <https://github.com/qiang-yu/sectun>\n";

static void printHelp() __attribute__ ((noreturn));

static void printHelp() {
    printf(_helpMessage, VERSION);
    exit(1);
}


/**
 *
 * @param args
 * @param filename
 * @return
 */
static int parseConfigFile(sectun_args_t *args, const char *filename) {

    // file buffer, we read whole file into memory
    char fileBuffer[ARGS_CONFIG_FILE_SIZE];
    memset(fileBuffer, '\0', ARGS_CONFIG_FILE_SIZE);

    // read conf file
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        err("fopen");
        errf("Can't open config file: %s", filename);
        return -1;
    }
    size_t readSize = fread(fileBuffer, sizeof(char), ARGS_CONFIG_FILE_SIZE, fp);
    fclose(fp);

    if (readSize <= 0) {
        errf("bad config file: %s", filename);
        return -1;
    }

    if (readSize >= ARGS_CONFIG_FILE_SIZE) {
        errf("too large config file: %s", filename);
        return -1;
    }

    // parse json file
    jsmn_parser jsonParser;
    jsmntok_t token[128]; /* We expect no more than 256 tokens */

    jsmn_init(&jsonParser);
    int tokenSize = jsmn_parse(&jsonParser, fileBuffer, strlen(fileBuffer), token,
                               sizeof(token) / sizeof(token[0]));
    if (tokenSize < 0) {
        errf("Failed to parse JSON: [%d] file [%s]", tokenSize, filename);
        return -1;
    }

    /* Assume the top-level element is an object */
    if (tokenSize < 1 || token[0].type != JSMN_OBJECT) {
        errf("no json object parsed [%s]", filename);
        return -1;
    }

    // read tokens
    int index;
    for (index = 1; index < tokenSize; index++) {
        jsmntok_t *tokenKey = &token[index];

        // a common string:value pair
        if (JSMN_STRING == tokenKey->type) {
            jsmntok_t *tokenValue = &token[index + 1];
            // skip value token
            index++;
            // get key:value pair
            const char *jsonKey = utilDupStr(fileBuffer + tokenKey->start, tokenKey->end - tokenKey->start);

            // kcp should be another json object
            if (0 == strcmp("kcp", jsonKey)) {
                // json object inside json
                if (JSMN_OBJECT != tokenValue->type ||
                    0 != sectunKcpParseConfig(tokenValue, fileBuffer, &(args->kcpConfig))) {
                    errf("bad kcp config format [%s]", filename);
                    return -1;
                }
                // skip kcp config tokens
                index += 2 * (tokenValue->size);
                continue;
            }

            const char *jsonValue = utilDupStr(fileBuffer + tokenValue->start, tokenValue->end - tokenValue->start);

            // set values into environment so shell script can use it
            if (0 != strcmp("encryptKey", jsonKey)) {
                if (-1 == setenv(jsonKey, jsonValue, 1)) {
                    err("setenv");
                    return -1;
                }
            }

            if (0 == strcmp("device", jsonKey)) {
                args->device = jsonValue;
                continue;
            }

            if (0 == strcmp("host", jsonKey)) {
                args->host = jsonValue;
                continue;
            }

            if (0 == strcmp("port", jsonKey)) {
                args->port = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("mtu", jsonKey)) {
                args->mtu = atoi(jsonValue);
                args->kcpConfig.mtu = args->mtu;
                continue;
            }

            if (0 == strcmp("mode", jsonKey)) {
                // default mode is server
                args->mode = SECTUN_MODE_SERVER;
                if (0 == strcmp("client", jsonValue)) {
                    args->mode = SECTUN_MODE_CLIENT;
                }
                continue;
            }

            if (0 == strcmp("encrypt", jsonKey)) {
                args->encrypt = jsonValue;
                continue;
            }

            if (0 == strcmp("encryptKey", jsonKey)) {
                args->encryptKey = jsonValue;
                continue;
            }

            if (0 == strcmp("userToken", jsonKey)) {
                args->userToken = jsonValue;
                continue;
            }

            if (0 == strcmp("userTokenList", jsonKey)) {
                args->userTokenList = jsonValue;
                continue;
            }

            if (0 == strcmp("heartbeatInterval", jsonKey)) {
                args->heartbeatInterval = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("heartbeatTimeout", jsonKey)) {
                args->heartbeatTimeout = atoi(jsonValue);
                continue;
            }

            if (0 == strcmp("transport", jsonKey)) {
                args->transport = jsonValue;
                continue;
            }

            if (0 == strcmp("upScript", jsonKey)) {
                args->upScript = jsonValue;
                continue;
            }

            if (0 == strcmp("downScript", jsonKey)) {
                args->downScript = jsonValue;
                continue;
            }

            // we do not use net, it would be set and used by shell script
            if (0 == strcmp("net", jsonKey)) {
                char *p = strchr(jsonValue, '/');
                if (p) *p = '\0';
                in_addr_t addr = inet_addr(jsonValue);
                if (addr == INADDR_NONE) {
                    errf("warning: invalid net IP in config file: %s", jsonValue);
                }
                args->netip = ntohl((uint32_t) addr);
                continue;
            }

            errf("unknow key [%s]", jsonKey);

        } else {
            errf("bad format config [%s]", filename);
            return -1;
        }

    }

    return 0;
}

/**
 *  give default args here
 * @param args
 */
static void loadDefaultArgs(sectun_args_t *args) {

    args->device = "tun0";
    args->host = "0.0.0.0";
    args->port = 1234;
    args->mtu = 1380;
    args->encryptKey = "test-password";
    args->encrypt = "none";
    args->heartbeatInterval = 30;
    args->heartbeatTimeout = 60;
    args->transport = "udp";

    // load default kcp config
    sectunKcpLoadDefaultConfig(&(args->kcpConfig));
}

int sectunArgParse(sectun_args_t *args, int argc, char **argv) {
    int ch;
    bzero(args, sizeof(sectun_args_t));
    while ((ch = getopt(argc, argv, "hs:c:v")) != -1) {
        switch (ch) {
            case 's':
                if (strcmp("start", optarg) == 0)
                    args->cmd = SECTUN_CMD_START;
                else if (strcmp("stop", optarg) == 0)
                    args->cmd = SECTUN_CMD_STOP;
                else if (strcmp("restart", optarg) == 0)
                    args->cmd = SECTUN_CMD_RESTART;
                else {
                    errf("unknown command %s", optarg);
                    printHelp();
                }
                break;
            case 'c':
                args->confFile = strdup(optarg);
                break;
            case 'v':
                log_verbose_mode = 1;
                break;
            default:
                printHelp();
        }
    }
    if (!args->confFile) {
        printHelp();
    }
    if (SECTUN_CMD_NONE == args->cmd) {
        printf("Error : missing -s start/stop/restart\n\n");
        printHelp();
    }
    loadDefaultArgs(args);
    return parseConfigFile(args, args->confFile);
}

void sectunArgDump(FILE *stream, sectun_args_t *args) {

    const char *sectunMode[3] = {"SECTUN_MODE_NONE", "SECTUN_MODE_SERVER", "SECTUN_MODE_CLIENT"};
    const char *sectunCmd[4] = {"SECTUN_CMD_NONE", "SECTUN_CMD_START", "SECTUN_CMD_STOP", "SECTUN_CMD_RESTART"};

    if (args->mode <= SECTUN_MODE_CLIENT) {
        fprintf(stream, "sectun_mode : [%s]\n", sectunMode[args->mode]);
    } else {
        fprintf(stream, "sectun_mode : [%d]\n", args->mode);
    }

    if (args->cmd <= SECTUN_CMD_RESTART) {
        fprintf(stream, "sectun_cmd : [%s]\n", sectunCmd[args->cmd]);
    } else {
        fprintf(stream, "sectun_cmd : [%d]\n", args->cmd);
    }

    fprintf(stream, "device : [%s]\n", args->device);
    fprintf(stream, "host : [%s]\n", args->host);
    fprintf(stream, "port : [%d]\n", args->port);
    fprintf(stream, "mtu : [%d]\n", args->mtu);
    fprintf(stream, "encrypt : [%s]\n", args->encrypt);
    fprintf(stream, "encryptKey : [%s]\n", args->encryptKey);

    struct in_addr in;
    in.s_addr = htonl((uint32_t) args->netip);
    fprintf(stream, "netip : [%s]\n", inet_ntoa(in));

    fprintf(stream, "userToken : [%s]\n", args->userToken);
    fprintf(stream, "userTokenList : [%s]\n", args->userTokenList);
    fprintf(stream, "heartbeatInterval : [%lu]\n", args->heartbeatInterval);
    fprintf(stream, "heartbeatTimeout : [%lu]\n", args->heartbeatTimeout);
    fprintf(stream, "transport : [%s]\n", args->transport);
    fprintf(stream, "upScript : [%s]\n", args->upScript);
    fprintf(stream, "downScript : [%s]\n", args->downScript);

    fprintf(stream, "confFile : [%s]\n", args->confFile);

    // dump other module
    fprintf(stream, "----------kcp config start----------\n");
    sectunKcpDumpConfig(stream, &(args->kcpConfig));
    fprintf(stream, "----------kcp config end  ----------\n");

}