#ifndef _UDP_H
#define _UDP_H

#include <string.h>
#include <stdlib.h>

#include "ipv4.h"

#define UDP_PROTOCOL_TYPE 17 
#define UDP_HEADER_SIZE 8 //bytes

typedef struct udp_layer udp_layer_t;


/* int udp_open(ipv4_addr_t dest, int port);
 * 
 * DESCRIPCIÓN:
 *   Esta función abre un puerto UDP que apunta a una cierta direccion
 *   ipv4, a x puerto.
 *
 * PARÁMETROS:
 *         'src': Local IPv4 address
 *    'src_port': Source port (local machine), of where we send the datagram from
 *
 * VALOR DEVUELTO:
 *   Devuelve 0 si la conexion de UDP se ha abierto correctamente.
 * 
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error. 
 */
udp_layer_t* udp_open(int src_port, char *file_conf, char *file_conf_route);

/* int udp_close(socket);
 * 
 * DESCRIPCIÓN:
 *   Esta función cierra el puerto UDP.
 *
 * PARÁMETROS:
 *      'socket':
 *
 * VALOR DEVUELTO:
 *   Devuelve 0 si la conexion de UDP se ha cerrado correctamente.
 * 
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error. 
 */
int udp_close(udp_layer_t* my_udp_iface);

/* int udp_send (ipv4_addr_t dst, unsigned char * payload, int payload_len);
 * 
 * DESCRIPCIÓN:
 *   Esta función empaquetará el datagrama, compuesto por la cabezera UDP y
 *   los datos a enviar
 *
 * PARÁMETROS:
 *         'dest':
 *      'payload':
 *  'payload_len':
 *
 * VALOR DEVUELTO:
 *   Devuelve 0 si la interfaz Ethernet se ha cerrado correctamente.
 * 
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error. 
 */
int udp_send(udp_layer_t *my_udp_layer, ipv4_addr_t dest, uint16_t dest_port, unsigned char *payload, int payload_len);

/* int udp_recv(datagram);
 * 
 * DESCRIPCIÓN:
 *   Esta función recivirá el datagrama y desempaquetará sus payload
 *
 * PARÁMETROS:
 *      'datagram':
 *
 * VALOR DEVUELTO:
 *   Devuelve 0 si se ha desempaquetado el datagrama correctamente.
 * 
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error. 
 */
int udp_rcv(udp_layer_t *my_udp_layer, ipv4_addr_t src, uint16_t *dest_port, unsigned char buffer[], int buf_len, long int timeout);

int random_port_generator(void);//Genera un numero de puerto aleatorio para mi máquina.

#endif /*_UDP_H */
