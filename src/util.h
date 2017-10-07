//
// Created by YuQiang on 2017-09-30.
//

#ifndef SECTUN_UTIL_H
#define SECTUN_UTIL_H

/**
 *  dump string
 *
 * @param str
 * @param len
 * @return
 */
char *utilDupStr(const char *str, int len);


/*
   RFC791
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version|  IHL  |Type of Service|          Total Length         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Identification        |Flags|      Fragment Offset    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Time to Live |    Protocol   |         Header Checksum       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       Source Address                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Destination Address                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/*
typedef struct {
    uint8_t ver;
    uint8_t tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t frag;
    uint8_t ttl;
    uint8_t proto;
    uint16_t checksum;
    uint32_t saddr;
    uint32_t daddr;
} ipv4_hdr_t;
*/

typedef struct {
    uint16_t sport;
    uint16_t dport;
    uint32_t seq;
    uint32_t ack;
    uint32_t not_interested;
    uint16_t checksum;
    uint16_t upt;
} tcp_hdr_t;

typedef struct {
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t checksum;
} udp_hdr_t;


// from OpenVPN
// acc is the changes (+ old - new)
// cksum is the checksum to adjust
#define ADJUST_CHECKSUM(acc, cksum) { \
  int _acc = acc; \
  _acc += (cksum); \
  if (_acc < 0) { \
    _acc = -_acc; \
    _acc = (_acc >> 16) + (_acc & 0xffff); \
    _acc += _acc >> 16; \
    (cksum) = (uint16_t) ~_acc; \
  } else { \
    _acc = (_acc >> 16) + (_acc & 0xffff); \
    _acc += _acc >> 16; \
    (cksum) = (uint16_t) _acc; \
  } \
}


#endif //SECTUN_UTIL_H
