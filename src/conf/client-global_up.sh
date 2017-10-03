#!/bin/sh

# This script will be executed when client is up. All key value pairs (except
# password) in sectun config file will be passed to this script as
# environment variables.

# Turn on IP forwarding
sysctl -w net.ipv4.ip_forward=1

# Configure IP address and MTU of VPN interface
ip addr add $net dev $device
ip link set $device mtu $mtu
ip link set $device up

# Turn on NAT over VPN
iptables -t nat -A POSTROUTING -o $device -j MASQUERADE
iptables -I FORWARD 1 -i $device -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -I FORWARD 1 -o $device -j ACCEPT

# Direct route to VPN server's public IP via current gateway
ip route add $host via $(ip route show 0/0 | sort -k 7 | head -n 1 | sed -e 's/.* via \([^ ]*\).*/\1/')

# Shadow default route using two /1 subnets
ip route add   0/1 dev $device
ip route add 128/1 dev $device

echo $0 done
