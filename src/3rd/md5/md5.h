#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>
#include <stddef.h>

void md5(const unsigned char *input, size_t ilen, unsigned char output[16]);

#endif
