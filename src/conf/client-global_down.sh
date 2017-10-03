#!/bin/sh

# This script will be executed when client is down. All key value pairs (except
# password) in sectun config file will be passed to this script as
# environment variables.

# Turn off IP forwarding
#sysctl -w net.ipv4.ip_forward=0

# turn off NAT over VPN
iptables -t nat -D POSTROUTING -o $device -j MASQUERADE
iptables -D FORWARD -i $device -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -D FORWARD -o $device -j ACCEPT

# Restore routing table
ip route del $host
ip route del   0/1
ip route del 128/1

echo $0 done
