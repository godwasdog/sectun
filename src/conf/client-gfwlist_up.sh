#!/bin/sh

# This script will be executed when client is up. All key value pairs (except
# password) in sectun config file will be passed to this script as
# environment variables.

## here config the ip set name
ipSetName=gfwlist
ruleName=gfwmark

# Turn on IP forwarding
sysctl -w net.ipv4.ip_forward=1

# Configure IP address and MTU of VPN interface
ip addr add $net dev $device
ip link set $device mtu $mtu
ip link set $device up

# Turn on NAT over VPN
iptables -t nat -A POSTROUTING -o $device -j MASQUERADE
iptables -I FORWARD 1 -i $device -j ACCEPT
iptables -I FORWARD 1 -o $device -j ACCEPT

# make tcp adjust to MTU
iptables -t mangle -I FORWARD 1 -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu

# Change routing table
ipset -N $ipSetName iphash
ipset -A $ipSetName 8.8.8.8
ipset -A $ipSetName 8.8.4.4

iptables -t mangle -N $ruleName
iptables -t mangle -A PREROUTING -j $ruleName
iptables -t mangle -A OUTPUT -j $ruleName
iptables -t mangle -A $ruleName -m set --match-set $ipSetName dst -j MARK --set-mark 0xfefe

echo "99 gfwroute" >> /etc/iproute2/rt_tables
ip route add default dev $device table gfwroute
ip rule add fwmark 0xfefe table gfwroute

echo $0 done
