#! /bin/bash
# for telling us that it is an executable.
#IP Client:
rawnetcc $1/ipv4_client ipv4_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c log.c $2 -DLOG_USE_COLOR
#IP Server:
rawnetcc $1/ipv4_server ipv4_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c log.c $2 -DLOG_USE_COLOR
#Arp client
rawnetcc $1/arp_client arp_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c log.c $2 -DLOG_USE_COLOR
#UDP client:
rawnetcc $1/udp_client udp_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c $2 -DLOG_USE_COLOR
#UDP server:
rawnetcc $1/udp_server udp_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c $2 -DLOG_USE_COLOR
# RIP server
rawnetcc $1/ripv2_server ripv2_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c $2 -DLOG_USE_COLOR

# $1 for a specific route ./ or /tmp
#   if nothing is specified it will try to compile onto / and fail
# $2 for -DDEBUG