//
// Created by YuQiang on 2017-09-17.
//

#ifndef SECTUN_LOG_H
#define SECTUN_LOG_H

#include <string.h>
#include <stdio.h>
#include <errno.h>

extern int log_verbose_mode;

/*
   err:    same as perror but with timestamp
   errf:   same as printf to stderr with timestamp and \n
   logf:   same as printf to stdout with timestamp and \n,
           and only enabled when verbose is on
   debugf: same as logf but only compiles with DEBUG flag
*/

#define __LOG(o, isVerbose, s...) do {                            \
  if (isVerbose || log_verbose_mode > 0) {                        \
    log_timestamp(o);                                             \
    fprintf(o, s);                                                \
    fprintf(o, "\n");                                             \
    fflush(o);                                                    \
  }                                                               \
} while (0)


#define logf(s...) __LOG(stdout, 0, s)
#define errf(s...) __LOG(stderr, 1, s)
#define err(s) log_perror_timestamp(s, __FILE__, __LINE__)

#ifdef DEBUG
#define debugf(s...) logf(s)
#else
#define debugf(s...)
#endif

void log_timestamp(FILE *out);

void log_perror_timestamp(const char *msg, const char *file, int line);

void log_print_hex_memory(void *mem, size_t len);

const char *log_hex_memory_32_bytes(const char *memory);

#endif //SECTUN_LOG_H
