//
// Created by YuQiang on 2017-09-30.
//

#include <stdlib.h>
#include <string.h>

#include "inc.h"
#include "log.h"
#include "util.h"

/**
 *  dup string for config values
 * @param str
 * @param len
 * @return
 */
char *utilDupStr(const char *str, int len) {
    static int isInit = 0;
    static char strArray[UTIL_DUP_STRING_MAX_SIZE];
    static char *pCur = NULL;
    static char *pEnd = NULL;
    if (0 == isInit) {
        isInit = 1;
        pCur = strArray;
        pEnd = pCur + UTIL_DUP_STRING_MAX_SIZE;
        memset(strArray, '\0', UTIL_DUP_STRING_MAX_SIZE);
    }
    if (pCur + len + 1 > pEnd) {
        errf("space use out");
        exit(1);
    }
    char *pStr = pCur;
    if (len > 0) {
        memcpy(pStr, str, len);
    }
    pCur += len + 1;
    return pStr;
}
