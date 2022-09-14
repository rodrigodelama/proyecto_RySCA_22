#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct arp_header
{
    uint8_t hw_type;
    uint8_t protocol_type; //IPv4
    uint8_t hw_size;
    uint8_t protocol_size;
    uint8_t opcode; //request (1) or reply (2)
    mac_addr_t sndr_MAC; // format XX:XX:XX:XX:XX:XX
    ipv4_addr_t sndr_IP; // format xxx.xxx.xxx.xxx, where x is any int [0,255]
    mac_addr_t target_MAC;
    ipv4_addr_t target_IP;
};
