# include "../eth/eth.h"
# include "../ipv4/ipv4.h"

# include <stdint.h>
# include <stdlib.h>    //Para funcion exit()
# include <string.h>    //Para funcion memset()
# include <arpa/inet.h> //Para htons, ntons y demas

#define MAC_STR_SIZE  17
#define HW_TYPE_ETH 1
#define PROT_TYPE_IPV4 0x0800
#define PROT_TYPE_ARP 0x0806
#define HW_SIZE_MAC_ADDR 6   //6 bytes las dir MAC
#define PROT_SIZE_IP_ADDR 4  //4 bytes las dir IP
#define OPCODE_REQUEST 1
#define OPCODE_REPLY 2
int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr);

mac_addr_t discovery_mac_addr; //MAC Address to be "discovered" by our ARP request
