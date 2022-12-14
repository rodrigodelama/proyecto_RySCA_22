#ifndef _IPv4_H
#define _IPv4_H

#include "eth.h"
#include "ipv4_dependencies.h"

#include <stdint.h>

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
void ipv4_addr_str ( ipv4_addr_t addr, char *str );


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
int ipv4_str_addr ( char *str, ipv4_addr_t addr );


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
uint16_t ipv4_checksum ( unsigned char *data, int len );

//ipv4 open, parámetros de entrada son los txt de ipv4_route_table
ipv4_layer_t* ipv4_open(char *file_conf, char *file_conf_route); //Devuelve el manejador de ipv4. 

int ipv4_close(ipv4_layer_t *iface);

int ipv4_send(ipv4_layer_t *layer, ipv4_addr_t dst, uint8_t protocol, unsigned char *payload, int payload_len);

int ipv4_recv(ipv4_layer_t *layer, uint8_t protocol, unsigned char buffer [], ipv4_addr_t sender, int buf_len,long int timeout);

#endif /*_IPv4_H */
