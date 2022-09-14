# include "../eth_base/eth.h"
# include "../ipv4_base/ipv4.h"
# include <stdint.h>

struct arp_header
{
    uint16_t hardware_type; //protocolo capa inferior (eth)
    uint16_t protocol_type; //protocolo capa superior (ipv4)
    uint8_t hw_size; //numero de bytes de las direcciones de la capa inferior (6 bytes en Eth)
    uint8_t protocol_size; //numero de bytes de las direcciones de la capa superior (4 bytes en IPV4)
    uint16_t opcode; //request = 1, reply = 2
    mac_addr_t src_eth_addr; //sender MAC address - format XX:XX:XX:XX:XX:XX
    ipv4_addr_t src_ipv4_addr; //sender IPv4 address - format xxx.xxx.xxx.xxx, where x is any int [0,255]
    mac_addr_t dest_eth_addr; //target MAC address
    ipv4_addr_t dest_ipv4_addr; //target IPv4 address
};


int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr)
{
    //ARP - Address Resolution Protocol

    //respond MAC when asked who has that IP Address
}

