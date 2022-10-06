#ifndef _UDP_H
#define _UDP_H

#include "ipv4.h"

/* int udp_open(ipv4_addr_t dest, int port);
 * 
 * DESCRIPCIÓN:
 *   Esta función abre un puerto UDP que apunta a una cierta direccion
 *   ipv4, a x puerto.
 *
 * PARÁMETROS:
 *         'src': Local IPv4 address
 *        'dest': IPv4 address provided to send datagram to
 *    'src_port': Source port (local machine), of where we send the datagram from
 *   'dest_port': Target port (target), to where we send the datagram to
 *
 * VALOR DEVUELTO:
 *   Devuelve 0 si la conexion de UDP se ha abierto correctamente.
 * 
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error. 
 */
int udp_open(ipv4_addr_t src, ipv4_addr_t dest, int src_port, int dest_port);


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
int udp_close(socket);

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
int udp_send(ipv4_addr_t dest, unsigned char * payload, int payload_len);

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
int udp_recv(datagram);

#endif /*_UDP_H */
