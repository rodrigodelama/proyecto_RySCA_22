#! /bin/bash
# This above tells bash that it is an executable

# $1 for -DEBUG

# Arp Client
rawnetcc /tmp/arp_client arp_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c log.c $1 -DLOG_USE_COLOR
# IP Client:
rawnetcc /tmp/ipv4_client ipv4_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c log.c $1 -DLOG_USE_COLOR
# IP Server:
rawnetcc /tmp/ipv4_server ipv4_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c log.c $1 -DLOG_USE_COLOR
# UDP Client:
rawnetcc /tmp/udp_client udp_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c $1 -DLOG_USE_COLOR
# UDP Server:
rawnetcc /tmp/udp_server udp_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c $1 -DLOG_USE_COLOR


# FINAL EXAM COMPILES below:

# ripv2_client
# rip2_server
# arp
# ipv4
# ipv4_config
# ipv4_route_table
# ripv2_route_table
# eth
# udp
# log

# RIP Client
rawnetcc /tmp/ripv2_client ripv2_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c ripv2_route_table.c eth.c udp.c log.c $1 -DLOG_USE_COLOR
# RIP Server
rawnetcc /tmp/ripv2_server ripv2_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c ripv2_route_table.c eth.c udp.c log.c $1 -DLOG_USE_COLOR
