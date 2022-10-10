#include "eth.h"

#define IPv4_ADDR_SIZE 4
#define IPv4_STR_MAX_LENGTH 16
#define IPV4_HDR_LEN 20

/* Logitud máxmima del nombre de un interfaz de red */
#define IFACE_NAME_MAX_LENGTH 32

typedef unsigned char ipv4_addr_t[IPv4_ADDR_SIZE];

/*ipv4_route_table---------------------------------------------------------------------------*/
/* Número de entradas máximo de la tabla de rutas IPv4 */
#define IPv4_ROUTE_TABLE_SIZE 256

typedef struct ipv4_route {
    ipv4_addr_t subnet_addr;
    ipv4_addr_t subnet_mask;
    char iface[32]; //Interfaz por la que mandaremos
    ipv4_addr_t gateway_addr; //siguiente salto, si esta en mi subred, este campo es 0.
} ipv4_route_t;

typedef struct ipv4_route_table {
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


/*--------------------------------------------------------------------------------------------*/
