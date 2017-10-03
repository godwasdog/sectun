//
// Created by YuQiang on 2017-09-17.
//

#include <time.h>

#include "inc.h"
#include "log.h"

// verbose mode would show all logs
int log_verbose_mode = 0;

void log_timestamp(FILE *out) {
    char datetime[24];
    time_t now;
    time(&now);
    int len = strftime(datetime, 24, "%Y-%m-%d %H:%M:%S ", localtime(&now));
    datetime[len] = '\0';
    fprintf(out, "%s", datetime);
}

void log_perror_timestamp(const char *msg, const char *file, int line) {
    log_timestamp(stderr);
    fprintf(stderr, "%s:%d ", file, line);
    perror(msg);
}

void log_print_hex_memory(void *mem, size_t len) {
    int i;
    unsigned char *p = (unsigned char *) mem;
    for (i = 0; i < len; i++) {
        printf("%02x ", p[i]);
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n");
}

const char *log_hex_memory_32_bytes(const char *memory) {
    int i;
    static unsigned char buffer[34];
    static char output[128];
    char *outptr = output;

    memset(buffer, '\0', sizeof(buffer));
    memset(output, '\0', sizeof(output));
    memcpy(buffer, memory, 32);

    for (i = 0; i < 32; i++) {
        sprintf(outptr, "%02x ", buffer[i]);
        outptr += 3;
    }
    // remove last space character
    *(--outptr) = '\0';

    return output;
}