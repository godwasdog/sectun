//
// Created by YuQiang on 2017-10-01.
//

#ifndef SECTUN_ENCRYPT_H
#define SECTUN_ENCRYPT_H

#include "itransport.h"

/**
 *  init encrypt system
 * @param encrypt
 * @param encryptKey
 * @return
 */
int sectunEncryptInit(const char *encrypt, const char *encryptKey);

/**
 *
 * @return
 */
struct itransport *const sectunGetEncryptTransport();

#endif //SECTUN_ENCRYPT_H
