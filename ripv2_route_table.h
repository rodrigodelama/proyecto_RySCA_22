#ifndef _RIPv2_TABLE_H
#define _RIPv2_TABLE_H

#include <stdio.h>

#include "ripv2.h"

/* Esta estructura almacena la información básica sobre la ruta a una subred. 
 * Incluye la dirección y máscara de la subred destino, el nombre del interfaz
 * de salida, y la dirección IP del siguiente salto.
 *
 * Utilice los métodos 'ipv4_route_create()' e 'ipv4_route_free()' para crear
 * y liberar esta estrucutra. Adicionalmente debe completar la implementación
 * del método 'ipv4_route_lookup()'.
 * 
 * Probablemente para construir una tabla de rutas de un protocolo de
 * encaminamiento sea necesario añadir más campos a esta estructura, así como
 * modificar las funciones asociadas.
 */
//we shant redeclare these
//typedef unsigned char ipv4_addr_t[];

/* ipv4_route_t * ipv4_route_create
 * ( ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw );
 * 
 * DESCRIPCIÓN: 
 *   Esta función crea una ruta RIPv2 con los parámetros especificados:
 *   dirección de subred, máscara, nombre de interfaz y dirección de siguiente
 *   salto.
 *
 *   Esta función reserva memoria para la estructura creada. Debe utilizar la
 *   función 'ipv4_route_free()' para liberar dicha memoria.
 *
 * PARÁMETROS:
 *   'subnet': Dirección IPv4 de la subred destino de la nueva ruta.
 *     'mask': Máscara de la subred destino de la nueva ruta.
 *    'iface': Nombre del interfaz empleado para llegar a la subred destino de
 *             la nueva  ruta. Debe tener una longitud máxima de 
 *             'IFACE_NAME_MAX_LENGTH' caracteres.
 *       'gw': Dirección IPv4 del encaminador empleado para llegar a la subred
 *             destino de la nueva ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la ruta creada.
 * 
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la ruta.
 */
ripv2_route_t * ripv2_route_create(ipv4_addr_t subnet, ipv4_addr_t mask, char* iface, ipv4_addr_t gw, uint32_t metric, long timerms);

/* int ripv2_route_lookup ( ripv2_route_t * route, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función indica si la dirección IPv4 especificada pertence a la
 *   subred indicada. En ese caso devuelve la longitud de la máscara de la
 *   subred.
 *
 * ************************************************************************
 * * Esta función NO está implementada, debe implementarla usted para que *
 * * funcione correctamente la función 'ipv4_route_table_lookup()'.       *
 * ************************************************************************
 * 
 * PARÁMETROS:
 *   'route': Ruta a la subred que se quiere comprobar.
 *    'addr': Dirección IPv4 destino.
 *
 * VALOR DEVUELTO:
 *   Si la dirección IPv4 pertenece a la subred de la ruta especificada, debe
 *   devolver un número positivo que indica la longitud del prefijo de
 *   subred. Esto es, el número de bits a uno de la máscara de subred.
 *   La función devuelve '-1' si la dirección IPv4 no pertenece a la subred
 *   apuntada por la ruta especificada.
 */
int ripv2_route_lookup ( ripv2_route_t * route, ipv4_addr_t addr );


/* void ripv2_route_print ( ripv2_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime la ruta especificada por la salida estándar.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea imprimir.
 */
void ripv2_route_print ( ripv2_route_t * route );

// printing distance vectors.
void ripv2_vector_print(vector_distancia_t * vector);

/* void ripv2_route_free ( ripv_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la ruta especificada, que
 *   ha sido creada con 'ripv2_route_create()'.
 *
 * PARÁMETROS:
 *   'route': Ruta cuya memoria se desea liberar.
 */
void ripv2_route_free ( ripv2_route_t * route );


/* Definción de la estructura opaca que modela una tabla de rutas RIPv2.
 * Las entradas de la tabla de rutas están indexadas, y dicho índice puede
 * tener un valor entre 0 y 'RIPv2_ROUTE_TABLE_SIZE - 1'. Esta
 * implementación no permite rutas duplicadas (e.g. la misma ruta con
 * diferentes distancias administrativas), así que antes de añadir una
 * nueva ruta debe comprobar que no existe previamente.
 *
 * Esta estructura nunca debe crearse directamente. En su lugar debe emplear
 * las funciones 'ripv2_route_table_create()' e 'ripv2_route_table_free()' para
 * crear y liberar dicha estructura, respectivamente.
 *
 * Una vez creada la tabla de rutas, utilice 'ripv2_route_table_get()' para
 * acceder a la ruta en una posición determinada. Además es posible añadir
 * ['ripv2_route_table_add()'] y borrar rutas ['ripv2_route_table_remove()'],
 * así como buscar una subred en particular ['ripv2_route_table_find()'].
 * 'ripv2_route_table_lookup()' es la función más importante de la tabla de
 * rutas ya que devuelve la ruta para llegar a la dirección RIPv2 destino
 * especificada.
 * 
 * Adicionalmente, las funciones 'ripv2_route_table_read()',
 * 'ripv2_route_table_write()' y 'ripv2_route_table_print()' permiten,
 * respectivamente, leer/escribir la tabla de rutas de/a un fichero, e
 * imprimirla por la salida estándar.
 */


/* ripv2_route_table_t * ripv2_route_table_create();
 * 
 * DESCRIPCIÓN: 
 *   Esta función crea una tabla de rutas RIPv2 vacía.
 *
 *   Esta función reserva memoria para la tabla de rutas creada, para
 *   liberarla es necesario llamar a la función 'ripv2_route_table_free()'.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la tabla de rutas creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la tabla de rutas.
 */
ripv2_route_table_t * ripv2_route_table_create();


/* int ripv2_route_table_add ( ripv2_route_table_t * table, ripv2_route_t * route );
 * DESCRIPCIÓN: 
 *   Esta función añade la ruta especificada en la primera posición libre de
 *   la tabla de rutas.
 *
 * PARÁMETROS:
 *   'table': Tabla donde añadir la ruta especificada.
 *   'route': Ruta a añadir en la tabla de rutas.
 * 
 * VALOR DEVUELTO:
 *   La función devuelve el indice de la posición [0, RIPv2_ROUTE_TABLE_SIZE-1]
 *   donde se ha añadido la ruta especificada.
 * 
 * ERRORES:
 *   La función devuelve '-1' si no ha sido posible añadir la ruta
 *   especificada.
 */
int ripv2_route_table_add ( ripv2_route_table_t * table, ripv2_route_t * route );


/* ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table, int index );
 *
 * DESCRIPCIÓN:
 *   Esta función borra la ruta almacenada en la posición de la tabla de rutas
 *   especificada.
 *   
 *   Esta función NO libera la memoria reservada para la ruta borrada. Para
 *   ello es necesario utilizar la función 'ripv2_route_free()' con la ruta
 *   devuelta.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea borrar una ruta.
 *   'index': Índice de la ruta a borrar. Debe tener un valor comprendido
 *            entre [0, RIPv2_ROUTE_TABLE_SIZE-1].
 * 
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta que estaba almacenada en la posición
 *   indicada antes de ser borrada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si la ruta no ha podido ser borrada, o no
 *   existía ninguna ruta en dicha posición.
 */
ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table, int index );


/* ripv2_route_t * ripv2_route_table_lookup ( ripv2_route_table_t * table, ripv2_addr_t addr );
 * 
 * DESCRIPCIÓN:
 *   Esta función devuelve la mejor ruta almacenada en la tabla de rutas para
 *   alcanzar la dirección IPv4 destino especificada.
 *
 *   Esta función recorre toda la tabla de rutas buscando rutas que contengan
 *   a la dirección IPv4 indicada. Para ello emplea la función
 *   'ripv2_route_lookup()'. De todas las rutas posibles se devuelve aquella
 *   con el prefijo más específico, esto es, aquella con la máscara de subred
 *   mayor.
 * 
 * PARÁMETROS:
 *   'table': Tabla de rutas en la que buscar la dirección IPv4 destino.
 *    'addr': Dirección IPv4 destino a buscar.
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta más específica para llegar a la dirección
 *   IPv4 indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no no existe ninguna ruta para alcanzar
 *   la dirección indicada, o si no ha sido posible realizar la búsqueda.
 */
ripv2_route_t * ripv2_route_table_lookup ( ripv2_route_table_t * table, ipv4_addr_t addr );


/* ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index );
 * 
 * DESCRIPCIÓN:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas especificada.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea obtener una ruta.
 *   'index': Índice de la ruta consultada. Debe tener un valor comprendido
 *            entre [0, RIPv2_ROUTE_TABLE_SIZE-1].
 * 
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' no existe ninguna ruta en dicha posición, o
 *   si no ha sido posible consultar la tabla de rutas.
 */ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index );


/* _route_table_find ( ripv2_route_table_t * table, ripv2_addr_t subnet, ripv2_addr_t mask );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el índice de la ruta para llegar a la subred
 *   especificada.
 *
 * PARÁMETROS:
 *    'table': Tabla de rutas en la que buscar la subred.
 *   'subnet': Dirección de la subred a buscar.
 *     'mask': Máscara de la subred a buscar.
 * 
 * VALOR DEVUELTO:
 *   Esta función devuelve la posición de la tabla de rutas donde se encuentra
 *   la ruta que apunta a la subred especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no se ha encontrado la ruta especificada o
 *   '-2' si no ha sido posible realizar la búsqueda.
 */
int ripv2_route_table_find( ripv2_route_table_t * table, ipv4_addr_t subnet, ipv4_addr_t mask );


/* void ripv2_route_table_free ( ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la tabla de rutas
 *   especificada, incluyendo todas las rutas almacenadas en la misma,
 *   mediante la función 'ripv2_route_free()'.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas a borrar.
 */
void ripv2_route_table_free ( ripv2_route_table_t * table );


/* int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función lee el fichero especificado y añade las rutas RIPv2
 *   estáticas leídas en la tabla de rutas indicada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero con rutas RIPv2 que se desea leer.
 *      'table': Tabla de rutas donde añadir las rutas leidas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas leidas y añadidas en la tabla, o
 *   '0' si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al leer el
 *   fichero de rutas.
 */
int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table );


/* void ripv2_route_output ( ripv2_route_t * route, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la ruta RIPv2
 *   especificada.
 *
 * PARÁMETROS:
 *      'route': Ruta a imprimir.
 *     'header': '0' para imprimir una línea con la cabecera de la ruta.
 *        'out': Salida por la que imprimir la ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si la ruta se ha impreso correctamente.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 */
int ripv2_route_output ( ripv2_route_t * route, int header, FILE * out, int route_index );


/* void ripv2_route_table_print ( ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida estándar la tabla de rutas RIPv2
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 */
void ripv2_route_table_print ( ripv2_route_table_t * table );


/* int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename );
 *
 * DESCRIPCIÓN:
 *   Esta función almacena en el fichero especificado la tabla de rutas RIPv2
 *   indicada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a almacenar.
 *   'filename': Nombre del fichero donde se desea almacenar la tabla de
 *               rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas almacenadas en el fichero de
 *   rutas, o '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir el
 *   fichero de rutas.
 */
int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename );

#endif /* _RIPv2_ROUTE_TABLE_H */
