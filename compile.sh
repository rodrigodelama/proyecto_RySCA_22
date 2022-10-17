#! /bin/bash

rawnetcc $1/ipv4_client ipv4_client.c arp.c ipv4.c ipv4_config.c ipv4_route_table.c eth.c udp.c -DDEBUG