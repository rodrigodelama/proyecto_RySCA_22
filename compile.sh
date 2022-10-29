#! /bin/bash 
# for telling us that it is an executable.
#Client:
rawnetcc $1/ipv4_client ipv4_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c -DDEBUG -DLOG_USE_COLOR
#Server:
rawnetcc $1/ipv4_server ipv4_server.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c -DDEBUG -DLOG_USE_COLOR
#Arp client
rawnetcc $1/arp_client arp_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c -DDEBUG -DLOG_USE_COLOR
#rawnetcc $1/arp_client arp_client.c arp.c ipv4.c eth.c log.c -DDEBUG -DLOG_USE_COLOR
#UDP client:
rawnetcc $1/udp_client udp_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c log.c -DDEBUG -DLOG_USE_COLOR

#El 1 al principio era para poder especificar la ruta del ejecutable