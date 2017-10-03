//
// Created by YuQiang on 2017-10-01.
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "3rd/aes/aes.h"
#include "3rd/md5/md5.h"

#include "inc.h"
#include "log.h"
#include "encrypt.h"

#ifdef DEBUG_ENCRYPT
#define debugEncrypt(s...) __LOG(stdout, DEBUG_ENCRYPT, s)
#else
#define debugEncrypt(s...)
#endif

typedef enum {
    ENCRYPT_NONE = 0,
    ENCRYPT_AES128CBC,
    ENCRYPT_END
} encrypt_method_t;

typedef struct {
    ssize_t (*encrypt)(char *output, const char *data, size_t len);

    ssize_t (*decrypt)(char *output, const char *data, size_t len);
} encrypt_handle_t;

static encrypt_handle_t _encryptHandleArray[ENCRYPT_END];

/**
 * encryption context
 *
 */
static struct {
    unsigned char encryptKey[ENCRYPT_KEY_SIZE];
    //this prog use zero iv,you should make sure first block of data contains a random/nonce data
    unsigned char encryptVector[ENCRYPT_VECTOR_SIZE];
    encrypt_method_t encryptMethod;
    char dataBuffer[DATA_BUFFER_SIZE];
} _encryptCtx;

// encrypt transport
static struct itransport _encryptTransport;
static int _isEncryptInit = 0;

/**
 *
 * @param output
 * @param data
 * @param len
 * @return
 */
static ssize_t dummyEncrypt(char *output, const char *data, size_t len) {
    memcpy(output, data, len);
    debugEncrypt("dummyEncrypt [%d] bytes", len);
    return len;
}

/**
 *
 * @param output
 * @param data
 * @param len
 * @return
 */
static ssize_t dummyDecrypt(char *output, const char *data, size_t len) {
    memcpy(output, data, len);
    debugEncrypt("dummyDecrypt [%d] bytes", len);
    return len;
}

/**
 *  padding for encryption
 *
 * @param data
 * @param data_len
 * @param padding_num
 * @return
 */
static ssize_t padding(char *data, size_t data_len, int padding_num) {
    int old_len = data_len;
    data_len += 1;
    if (data_len % padding_num != 0) {
        data_len = (data_len / padding_num) * padding_num + padding_num;
    }
    data[data_len - 1] = (data_len - old_len);
    return data_len;
}

/**
 * de-padding for decryption
 *
 * @param data
 * @param data_len
 * @param padding_num
 * @return
 */
static ssize_t dePadding(const char *data, size_t data_len, int padding_num) {
    if ((uint8_t) data[data_len - 1] > padding_num) return -1;
    data_len -= (uint8_t) data[data_len - 1];
    if (data_len < 0) {
        return -1;
    }
    return data_len;
}

/**
 * encryption
 *
 * @param output
 * @param data
 * @param len
 * @return
 */
static ssize_t cipher_aes128cbc_encrypt(char *output, const char *data, size_t len) {

    static unsigned char dataBuffer[DATA_BUFFER_SIZE];

    if (len > DATA_BUFFER_SIZE - DATA_PADDING_SIZE) {
        errf("len [%d] is larger than DATA_BUFFER_SIZE - DATA_PADDING_SIZE [%d - %d]", len,
             DATA_BUFFER_SIZE, DATA_PADDING_SIZE);
        exit(-1);
    }

    // copy data here not efficient
    memcpy(dataBuffer, data, len);

    if ((len = padding(dataBuffer, len, 16)) < 0) {
        return -1;
    }

    AES_CBC_encrypt_buffer((unsigned char *) output, (unsigned char *) dataBuffer, len,
                           _encryptCtx.encryptKey, _encryptCtx.encryptVector);
    debugEncrypt("cipher_aes128cbc_encrypt [%d] bytes", len);
    return len;
}

/**
 * decryption
 *
 * @param output
 * @param data
 * @param len
 * @return
 */
static ssize_t cipher_aes128cbc_decrypt(char *output, const char *data, size_t len) {

    if (len % 16 != 0) {
        errf("len [%d] % 16 != 0", len);
        return -1;
    }

    AES_CBC_decrypt_buffer((unsigned char *) output, (unsigned char *) data, len,
                           _encryptCtx.encryptKey, _encryptCtx.encryptVector);
    if ((len = dePadding(output, len, 16)) < 0) {
        debugEncrypt("cipher_aes128cbc_decrypt dePadding [%d] bytes", len);
        return -1;
    }
    debugEncrypt("cipher_aes128cbc_decrypt dePadding [%d] bytes", len);
    return len;
}

static ssize_t encryptWriteData(char *buffer, size_t len) {

    // we do encryption here
    len = _encryptHandleArray[_encryptCtx.encryptMethod].encrypt(_encryptCtx.dataBuffer, buffer, len);
    if (len > 0) {
        return _encryptTransport.forwardWrite(_encryptCtx.dataBuffer, len);
    }
    return len;
}

static ssize_t encryptOnRead(char *buffer, size_t len) {
    // we do decryption here
    len = _encryptHandleArray[_encryptCtx.encryptMethod].decrypt(_encryptCtx.dataBuffer, buffer, len);
    if (len > 0) {
        _encryptTransport.forwardRead(_encryptCtx.dataBuffer, len);
    }
}

static ssize_t encryptFinishOnRead(size_t totalLen) {
    if (NULL != _encryptTransport.forwardReadFinish) {
        _encryptTransport.forwardReadFinish(totalLen);
    }
}

/**
 *
 * @param transport
 */
static void encryptSetNextLayerTransport(struct itransport *transport) {
    assert(NULL != transport);
    _encryptTransport.forwardWrite = transport->writeData;
    transport->forwardRead = encryptOnRead;
    transport->forwardReadFinish = encryptFinishOnRead;
}

/**
 *
 * @param encrypt
 * @param encryptKey
 * @return
 */
int sectunEncryptInit(const char *encrypt, const char *encryptKey) {

    assert(NULL != encrypt);
    assert(NULL != encryptKey);
    assert(0 == _isEncryptInit);

    // init context
    memset(&_encryptCtx, 0, sizeof(_encryptCtx));

    // we generate encrypt key here
    md5(encrypt, ENCRYPT_KEY_SIZE, _encryptCtx.encryptKey);

    // do not use encrypt if encrypt is none
    if (0 == strcmp("none", encrypt)) {
        _encryptCtx.encryptMethod = ENCRYPT_NONE;
    } else if (0 == strcmp("aes-128-cbc", encrypt)) {
        _encryptCtx.encryptMethod = ENCRYPT_AES128CBC;
    } else {
        // default use aes-128-cbc
        _encryptCtx.encryptMethod = ENCRYPT_AES128CBC;
    }

    debugEncrypt("Use Encrypt [%d] [%s]", _encryptCtx.encryptMethod, encrypt);

    // setup encryp/decrypt handle
    memset(&_encryptHandleArray, 0, sizeof(_encryptHandleArray));
    // none
    _encryptHandleArray[ENCRYPT_NONE].encrypt = dummyEncrypt;
    _encryptHandleArray[ENCRYPT_NONE].decrypt = dummyDecrypt;
    // aes-128-cbc
    _encryptHandleArray[ENCRYPT_AES128CBC].encrypt = cipher_aes128cbc_encrypt;
    _encryptHandleArray[ENCRYPT_AES128CBC].decrypt = cipher_aes128cbc_decrypt;


    // init encrypt transport
    _encryptTransport = __dummyTransport;
    _encryptTransport.writeData = encryptWriteData;
    _encryptTransport.setNextLayer = encryptSetNextLayerTransport;

    // finish init
    _isEncryptInit = 1;
    return 0;
}

/**
 *
 * @return
 */
struct itransport *const sectunGetEncryptTransport() {
    assert(_isEncryptInit > 0);
    return &_encryptTransport;
}
