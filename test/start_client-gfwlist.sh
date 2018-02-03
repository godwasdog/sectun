#!/bin/sh

############################# these code only for testing purpose #####################
## set default route
ip route add   0/1 dev eth0
ip route add   128/1 dev eth0

## add baidu ip to list
ipset -A  gfwlist  14.215.177.38

############################# these code only for testing purpose #####################






./sectun -c client-gfwlist-config.json -s start -v






############################# these code only for testing purpose #####################

## remove default route
ip route del   0/1 dev eth0
ip route del   128/1 dev eth0

############################# these code only for testing purpose #####################