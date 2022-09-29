#ifndef _IPv4_H
#define _IPv4_H

#include "../eth/eth.h"
#include "../ipv4_route_table/ipv4_route_table.h"
#include <stdint.h>

#define IPv4_ADDR_SIZE 4
#define IPv4_STR_MAX_LENGTH 16

typedef unsigned char ipv4_addr_t [IPv4_ADDR_SIZE];

typedef struct ipv4_layer {
    eth_iface_t * iface; /*Manejador de interfaz eth*/
    ipv4_addr_t addr; 
    ipv4_addr_t netmask; 
    ipv4_route_table_t * routing_table;
} ipv4_layer_t ;

/* Dirección IPv4 a cero "0.0.0.0" */
extern ipv4_addr_t IPv4_ZERO_ADDR;

/* Logitud máxmima del nombre de un interfaz de red */
#define IFACE_NAME_MAX_LENGTH 32


/* void ipv4_addr_str ( ipv4_addr_t addr, char* str );
 *
 * DESCRIPCIÓN:
 *   Esta función genera una cadena de texto que representa la dirección IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *   'addr': La dirección IP que se quiere representar textualente.
 *    'str': Memoria donde se desea almacenar la cadena de texto generada.
 *           Deben reservarse al menos 'IPv4_STR_MAX_LENGTH' bytes.
 */
void ipv4_addr_str ( ipv4_addr_t addr, char* str );


/* int ipv4_str_addr ( char* str, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función analiza una cadena de texto en busca de una dirección IPv4.
 *
 * PARÁMETROS:
 *    'str': La cadena de texto que se desea procesar.
 *   'addr': Memoria donde se almacena la dirección IPv4 encontrada.
 *
 * VALOR DEVUELTO:
 *   Se devuelve 0 si la cadena de texto representaba una dirección IPv4.
 *
 * ERRORES:
 *   La función devuelve -1 si la cadena de texto no representaba una
 *   dirección IPv4.
 */
int ipv4_str_addr ( char* str, ipv4_addr_t addr );


/*
 * uint16_t ipv4_checksum ( unsigned char * data, int len )
 *
 * DESCRIPCIÓN:
 *   Esta función calcula el checksum IP de los datos especificados.
 *
 * PARÁMETROS:
 *   'data': Puntero a los datos sobre los que se calcula el checksum.
 *    'len': Longitud en bytes de los datos.
 *
 * VALOR DEVUELTO:
 *   El valor del checksum calculado.
 */
uint16_t ipv4_checksum ( unsigned char * data, int len );
//ipv4 open, parámetros de entrada son los txt de ipv4_route_table
ipv4_layer_t* ipv4_open(char * file_conf, char * file_conf_route); //Devuelve el manejador de ipv4. 

int ipv4_send(ipv4_layer_t iface, ipv4_addr_t ip_addr_dest); //Devuelve nº de bytes enviados.

int ipv4_recv(ipv4_layer_t iface, ipv4_addr_t ip_addr_dest) //Devuelve nº bytes recibidos.

#endif /*_IPv4_H */
