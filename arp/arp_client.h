# include "../eth_base/eth.h"
# include "../ipv4_base/ipv4.h"
# include <stdint.h>

int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr);
