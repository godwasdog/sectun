
sectun package format

--------------------------------------------------------------------------------------------------
|                                          pppoe-wan (1480)                                      |
--------------------------------------------------------------------------------------------------
| IPV4(20) | UDP(8) | encrypt([16] padding) | heartbeat(1) | auth(8+4) | Kcp(24) | payload(1396) |
--------------------------------------------------------------------------------------------------

# MTU of sectun device. Use the following formula to calculate:
#     1480 (pppoe-wan) - 20 (IPv4, or 40 for IPv6) - 8 (UDP) - 16(encrypt padding) - 1(heartbeat) - 12(auth 8+4) - 24 (kcp)

we use mtu 1380 is a good choice

transport configuration can be combination like these

1. kcp+auth+heartbeat+encrypt+udp
2. auth+heartbeat+encrypt+udp
3. auth+encrypt+udp
4. auth+udp
5. udp

make sure heartbeat is before encrypt, so encrpyt would encrypt heartbeat data as well


Encryption can choose

1. none
2. aes-128-cbc
3. chacha20







