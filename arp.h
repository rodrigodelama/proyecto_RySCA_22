#include "eth.h"
#include "ipv4.h"

#include <stdint.h>
#include <stdlib.h>    //Para funcion exit()
#include <string.h>    //Para funcion memset()
#include <arpa/inet.h> //Para htons, ntons y demas

#define MAC_STR_SIZE  17
#define HW_TYPE_ETH 1
#define PROT_TYPE_IPV4 0x0800
#define PROT_TYPE_ARP 0x0806
#define HW_SIZE_MAC_ADDR 6   //6 bytes las dir MAC
#define PROT_SIZE_IP_ADDR 4  //4 bytes las dir IP
#define OPCODE_REQUEST 1
#define OPCODE_REPLY 2

int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr, ipv4_addr_t my_ip_addr);

struct arp_header
{
    //constants by the "defines"
    uint16_t hardware_type; //protocolo capa inferior (eth)
    uint16_t protocol_type; //protocolo capa superior (ipv4)
    uint8_t hw_size; //numero de bytes de las direcciones de la capa inferior (6 bytes en eth)
    uint8_t protocol_size; //numero de bytes de las direcciones de la capa superior (4 bytes en IPv4)

    uint16_t opcode; //request = 1, reply = 2
    mac_addr_t src_MAC_addr; //sender MAC address - format XX:XX:XX:XX:XX:XX
    ipv4_addr_t src_IPv4_addr; //sender IPv4 address - format xxx.xxx.xxx.xxx, where x is any int [0,255]
    mac_addr_t dest_MAC_addr; //target MAC address
    ipv4_addr_t dest_IPv4_addr; //target IPv4 address
};
