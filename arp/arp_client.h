# include "../eth_base/eth.h"
# include "../ipv4_base/ipv4.h"
# include <stdint.h>
# include <stdlib.h>//Para funcion exit().
# include <string.h>//Para funcion memset().
# include <arpa/inet.h>//Para htons, ntons y demas.

//extern mac_addr_t MAC_BCAST_ADDR ;

int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr);
