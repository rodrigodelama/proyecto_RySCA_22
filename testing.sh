#! /bin/bash

# $1 for parameter -DEBUG

# RIP Client
rawnetcc /tmp/ripv2_client ripv2_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c ripv2_route_table.c eth.c udp.c log.c $1 -DLOG_USE_COLOR
# RIP Server
rawnetcc /tmp/ripv2_server ripv2_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c ripv2_route_table.c eth.c udp.c log.c $1 -DLOG_USE_COLOR

cp *txt /tmp/
