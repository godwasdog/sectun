#!/bin/sh

# This script will be executed when client is down. All key value pairs (except
# password) in sectun config file will be passed to this script as
# environment variables.

# Turn off IP forwarding
# sysctl -w net.ipv4.ip_forward=0

ipSetName=gfwlist
ruleName=gfwmark

# turn off NAT over VPN
iptables -t nat -D POSTROUTING -o $device -j MASQUERADE
iptables -D FORWARD -i $device -j ACCEPT
iptables -D FORWARD -o $device -j ACCEPT

iptables -t mangle -D FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu

# Change routing table
## ip rule del table gfwroute
iptables -t mangle -D PREROUTING -j $ruleName
iptables -t mangle -D OUTPUT -j $ruleName
iptables -t mangle -D $ruleName -m set --match-set $ipSetName dst -j MARK --set-mark 0xfefe
iptables -t mangle -X $ruleName

ipset -X $ipSetName

ip rule del fwmark 0xfefe table gfwroute

## remove gfwroute
sed -i '/gfwroute/d' /etc/iproute2/rt_tables

echo $0 done
