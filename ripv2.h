#ifndef _RIPV2_H
#define _RIPV2_H

#include "udp.h"
#define AF_INET 2 
/* ripv2 ---------------------------------------------------------------------------------------*/
/* Estructura de las entradas de vectores de distancia de ripv2 */
typedef struct entrada_rip
{
    uint16_t familia_dirs;
    uint16_t etiqueta_ruta;
    ipv4_addr_t subred;
    ipv4_addr_t subnet_mask;
    ipv4_addr_t next_hop;
    uint32_t metric; // (4bytes)
} entrada_rip_t;

/* Estructura del mensaje de la interfaz ripv2 */
typedef struct ripv2_msg
{
    uint8_t type; //1=request 2=response
    uint8_t version; //version 2 always 0x02
    uint16_t dominio_encaminamiento; //todo a 0 
    entrada_rip_t vectores_distancia[25]; //(20 bytes) ??
} ripv2_msg_t;




/*--------------------------------------------------------------------------------------------*/

#endif
