#include "eth.h"

#define IPv4_ADDR_SIZE 4
#define IPv4_STR_MAX_LENGTH 16
#define IPV4_HDR_LEN 20

/* Logitud máxmima del nombre de un interfaz de red */
#define IFACE_NAME_MAX_LENGTH 32

#define UDP_PROTOCOL 17
#define VERSION_AND_LENGTH 0x45
#define ID 0x8397
#define TTL_DEF 64

typedef unsigned char ipv4_addr_t[IPv4_ADDR_SIZE];

/*ipv4_route_table---------------------------------------------------------------------------*/
/* Número de entradas máximo de la tabla de rutas IPv4 */
#define IPv4_ROUTE_TABLE_SIZE 256

typedef struct rip_route {
    ipv4_addr_t subnet_addr;
    ipv4_addr_t subnet_mask;
    char iface[32]; //Interfaz por la que mandaremos
    ipv4_addr_t gateway_addr; //siguiente salto, si esta en mi subred, este campo es 0.
} rip_route_t;

typedef struct rip_route_table {
  ipv4_route_t * routes[IPv4_ROUTE_TABLE_SIZE];
} ipv4_route_table_t;
/*--------------------------------------------------------------------------------------------*/

/*ipv4----------------------------------------------------------------------------------------*/
/* Estructura del manejador de la interfaz ivp4 */
typedef struct ipv4_layer
{
    eth_iface_t * iface; /*Manejador de interfaz eth*/
    ipv4_addr_t addr; //my address
    ipv4_addr_t netmask; //the networks netmask
    ipv4_route_table_t * routing_table; //my routing table
} ipv4_layer_t;

/* Estructura de la cabecera de ipv4 */
struct ipv4_header
{
  uint8_t version_and_length; //Default value = VERSION_AND_LENGTH -> 0x45
  uint8_t service_type; //This field to zeros.
  uint16_t total_length; //total payload that is being used.
  uint16_t identification; //Set to a number by default that we like.
  uint16_t frag_flags; //Set to 0 as we don't fragmentate.
  uint8_t ttl; //Set to 64
  uint8_t protocol; //UDP 
  uint16_t checksum; //returned value from checksum() function.
  ipv4_addr_t src_ip;
  ipv4_addr_t dest_ip;
  unsigned char payload[1460]; // 1500 MTU - 20cab eth - 20cab IP = 1460
};
/*--------------------------------------------------------------------------------------------*/
