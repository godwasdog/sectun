/**
 *
 * shell.c
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inc.h"
#include "log.h"
#include "shell.h"

int sectunShellRun(const char *script) {
    char *buf;
    int r;

    if (script == NULL || script[0] == 0) {
        errf("warning: script not set");
        return 0;
    }
    int bufSize = strlen(script) + 8;
    buf = malloc(bufSize);
    memset(buf, '\0', bufSize);

    sprintf(buf, "sh %s", script);

    logf("executing %s", script);
    if (0 != (r = system(buf))) {
        free(buf);
        errf("script %s returned non-zero return code: %d", script, r);
        return -1;
    }
    free(buf);
    return 0;
}


