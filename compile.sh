#! /bin/bash 
# for telling us that it is an executable.

rawnetcc $1/ipv4_client ipv4_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c -DDEBUG -DLOG_USE_COLOR

#El 1 al principio era para poder especificar la ruta del ejecutable? -> SI