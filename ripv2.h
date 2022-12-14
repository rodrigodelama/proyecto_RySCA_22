#ifndef _RIPV2_H
#define _RIPV2_H

#include "udp.h"
#include "global_dependencies.h"
//#define AF_INET 2  ya estaba definido

#define RIPv2_PORT 520
#define RECEPTION_TIMER 180000
#define RIPv2_ROUTE_TABLE_SIZE 25
#define LEN_PAYLOAD_RIP 1400
#define RIPv2_MESSAGE_HEADER_SIZE 4 // fam_dirs (1 byte) + uint8_t version (1 byte) + dominio_encaminamiento (2 bytes) = 4 bytes; 
#define RIPv2_DISTANCE_VECTOR_ENTRY_SIZE 20
#define RIPv2_REQUEST 1
#define RIPv2_RESPONSE 2

/* ripv2 ---------------------------------------------------------------------------------------*/
/* Estructura de las entradas de vectores de distancia de ripv2 */
//extern ipv4_addr_t IPv4_ZERO_ADDR;
typedef struct ripv2_route {
    ipv4_addr_t subnet_addr;
    ipv4_addr_t subnet_mask;
    char iface[32]; //Interfaz por la que mandaremos
    ipv4_addr_t gateway_addr; //siguiente salto, si esta en mi subred, este campo es 0.
    uint32_t metric ;
    timerms_t timer_ripv2;
} ripv2_route_t;

typedef struct ripv2_route_table {
  ripv2_route_t * routes[IPv4_ROUTE_TABLE_SIZE]; //max is 256
} ripv2_route_table_t;

typedef struct vector_distancia //vectores distancia
{
    uint16_t familia_dirs;//En RFC 2453, familia dirs debe ser 0, 
    uint16_t etiqueta_ruta;
    ipv4_addr_t subred;
    ipv4_addr_t subnet_mask;
    ipv4_addr_t next_hop;
    uint32_t metric; // (4bytes)
} vector_distancia_t;

//Si next hop == 0.0.0.0, el que lo reciba (si se guarda la ruta), pondrá como siguiente salto la dirección ip de origen

/* Estructura del mensaje de la interfaz ripv2 */
typedef struct ripv2_msg
{
    uint8_t type; //1=request 2=response
    uint8_t version; //version 2 always 0x02
    uint16_t dominio_encaminamiento; //todo a 0 
    vector_distancia_t vectores_distancia[25]; //(20 bytes) es el tamaño de cada "vector distancia", lo que son 25 el el número de entradas máximo en un datagrama.
} ripv2_msg_t;


/*--------------------------------------------------------------------------------------------*/

#endif
