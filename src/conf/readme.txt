
sectun package format

-----------------------------------------------------------------
|                   pppoe-wan (1480)                            |
-----------------------------------------------------------------
| IPV4 (20) | UDP (8) | sectun (32) | Kcp (24) | payload (1396) |
-----------------------------------------------------------------

# MTU of sectun device. Use the following formula to calculate:
#     1480 (pppoe-wan) - 20 (IPv4, or 40 for IPv6) - 8 (UDP) - 32(sectun keep) - 24 (kcp)

we use mtu 1380 is a good choice


transport configuration can be combination like these

1. kcp+heartbeat+encrypt+udp
2. heartbeat+encrypt+udp
3. encrypt+udp
4. udp

make sure heartbeat is before encrypt, so encrpyt would encrypt heartbeat data as well



Encryption can be choose

1. none
2. aes-128-cbc
3. chacha20







