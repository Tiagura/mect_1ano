#! /bin/bash

ip link add link dns-eth1 name dns-eth1.100 type vlan id 100
ip addr add dev dns-eth1.100 10.0.1.1/24
ip link set dev dns-eth1.100 up
ip addr flush dev dns-eth1

dnsmasq \
                   --dhcp-range=10.0.1.10,10.0.1.50 \
                   --dhcp-sequential-ip \
                   --dhcp-option=option:router,10.0.1.254 \
                   --no-resolv \
                   --txt-record=does.it.work,yes \
                   --bind-interfaces \
                   --except-interface=lo --interface=dns-eth1.100 \
                   --dhcp-leasefile=/tmp/nfv-dhcp-vlan100.leases \
                   --log-facility=/tmp/nfv-dhcp-vlan100.log \
                   --pid-file=/run/dnsmasq-vlan100.pid \
                   --conf-file=
  
ip link add link dns-eth1 name dns-eth1.200 type vlan id 200
ip addr add dev dns-eth1.200 10.0.2.1/24
ip link set dev dns-eth1.200 up
ip addr flush dev dns-eth1

dnsmasq \
                   --dhcp-range=10.0.2.10,10.0.2.50 \
                   --dhcp-sequential-ip \
                   --dhcp-option=option:router,10.0.2.254 \
                   --no-resolv \
                   --txt-record=does.it.work,yes \
                   --bind-interfaces \
                   --except-interface=lo --interface=dns-eth1.200 \
                   --dhcp-leasefile=/tmp/nfv-dhcp-vlan200.leases \
                   --log-facility=/tmp/nfv-dhcp-vlan200.log \
                   --pid-file=/run/dnsmasq-vlan200.pid \
                   --conf-file=
                   
iptables -t nat -A PREROUTING -i dns-eth1.100 -p udp --dport 53 -j DNAT --to-destination 10.0.1.1
iptables -t nat -A PREROUTING -i dns-eth1.100 -p tcp --dport 53 -j DNAT --to-destination 10.0.1.1
iptables -t nat -A PREROUTING -i dns-eth1.200 -p udp --dport 53 -j DNAT --to-destination 10.0.2.1
iptables -t nat -A PREROUTING -i dns-eth1.200 -p tcp --dport 53 -j DNAT --to-destination 10.0.2.1
