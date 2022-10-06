#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "arp.h"

#include <stdio.h>
#include <stdlib.h>
#define UDP_PROTOCOL 17
#define VERSION_AND_LENGTH 0x45
#define ID 0x8397
#define TTL_DEF 64
//Cuando capturemos ip, hacerlo en el que envía la trama, dado que en lightning descarta las tramas con el checksum mal.   
/* Dirección IPv4 a cero: "0.0.0.0" */
ipv4_addr_t IPv4_ZERO_ADDR = { 0, 0, 0, 0 };

/* Estructura del manejador del interfaz ivp4 */

/* Estructura de la capa de ipv4 */
struct ipv4_layer {
    eth_iface_t * iface; /*Manejador de interfaz eth*/
    ipv4_addr_t addr; 
    ipv4_addr_t netmask; 
    ipv4_route_table_t * routing_table;
};

struct ipv4_header {
  uint8_t version_and_length;//Default value = VERSION_AND_LENGTH -> 0x45
  uint8_t service_type;//This field to zeros.
  uint16_t total_length;//total payload that is being used.
  uint16_t identification;//Set to a number by default that we like.
  uint16_t frag_flags;//Set to 0 as we don't fragmentate.
  uint8_t ttl; //Set to 64
  uint8_t protocol; //UDP 
  uint8_t checksum;//returned value from checksum() function.
  ipv4_addr_t src_ip;
  ipv4_addr_t dest_ip;
  unsigned char payload[1480];// 1500 ETH - 20 cab IP = 1480.
};

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
void ipv4_addr_str ( ipv4_addr_t addr, char* str )
{
  if (str != NULL) {
    sprintf(str, "%d.%d.%d.%d",
            addr[0], addr[1], addr[2], addr[3]);
  }
}


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
int ipv4_str_addr ( char* str, ipv4_addr_t addr )
{
  int err = -1;

  if (str != NULL) {
    unsigned int addr_int[IPv4_ADDR_SIZE];
    int len = sscanf(str, "%d.%d.%d.%d", 
                    &addr_int[0], &addr_int[1], 
                    &addr_int[2], &addr_int[3]);

    if (len == IPv4_ADDR_SIZE) {
      int i;
      for (i=0; i<IPv4_ADDR_SIZE; i++) {
        addr[i] = (unsigned char) addr_int[i];
      }
      
      err = 0;
    }
  }
  
  return err;
}


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
uint16_t ipv4_checksum ( unsigned char * data, int len )
{
  int i;
  uint16_t word16;
  unsigned int sum = 0;
    
  /* Make 16 bit words out of every two adjacent 8 bit words in the packet
   * and add them up */
  for (i=0; i<len; i=i+2) {
    word16 = ((data[i] << 8) & 0xFF00) + (data[i+1] & 0x00FF);
    sum = sum + (unsigned int) word16;	
  }

  /* Take only 16 bits out of the 32 bit sum and add up the carries */
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /* One's complement the result */
  sum = ~sum;

  return (uint16_t) sum;
}

ipv4_layer_t* ipv4_open(char * file_conf, char * file_conf_route)
{

  /* 1. Crear layer -> routing_table */
   ipv4_layer_t * ipv4_layer = (ipv4_layer_t*) malloc(sizeof(ipv4_layer_t));
  if (ipv4_layer == NULL) {
    fprintf(stderr, "eth_open(): ERROR en malloc()\n");
    return NULL;
  }
  char iface_name[32];
/* 2. Leer direcciones y subred de file_conf */
  if (ipv4_config_read( file_conf, iface_name, ipv4_layer->addr, ipv4_layer->netmask) != 0){
    fprintf(stderr,"ERROR: file could not be opened correctly.\n");
    exit(-1);
  }
  //we can use in this case "=" instead of memcpy().
  
  /*La función devuelve '0' si el fichero de configuración se ha leido
    correctamente.*/
/* 3. Leer tabla de reenvío IP de file_conf_route */
  if(ipv4_route_table_read (file_conf_route, ipv4_layer->routing_table) != 0)
  {
    fprintf(stderr,"ERROR: file could not be opened correctly.\n");
    exit(-1);
  }
/* 4. Inicializar capa Ethernet con eth_open() */
  //FIXME:
  ipv4_layer->iface = eth_open(iface_name); //Returns eth interface controller  
  
  
  return ipv4_layer;

  //Guardamos el manejador en el campo de "iface".

}

int ipv4_close ( ipv4_layer_t * iface_ipv4 )
{
  int err = -1;

  if (iface_ipv4 != NULL)
  {
    err = eth_close(iface_ipv4->iface);
    free(iface_ipv4);
  }
  return err;
}

//SEND 6 RECEIVE:
int ipv4_send (ipv4_layer_t * layer, ipv4_addr_t dst, uint8_t protocol,unsigned char * payload, int payload_len)
{
  /* Comprobar parámetros */
  if (layer == NULL) {
    fprintf(stderr, "ipv4_send(): ERROR: ipv4_layer == NULL\n");
    return -1;
  }
  /* Crear el paquete IPv4 y rellenar todos los campos */
  struct ipv4_header ipv4_header_t;
  memset(&ipv4_header_t, 0, sizeof(struct ipv4_header)); //Relleno la zona de memoria que guarda nuestra cabecera IP con 0s
  ipv4_header_t.version_and_length= (uint8_t)VERSION_AND_LENGTH;
  ipv4_header_t.service_type=0;
  ipv4_header_t.total_length = (uint16_t) payload_len;
  ipv4_header_t.identification= (uint16_t) ID;
  ipv4_header_t.frag_flags= (uint16_t) 0;
  ipv4_header_t.ttl= (uint8_t) TTL_DEF;
  ipv4_header_t.protocol= (uint8_t)protocol;
  ipv4_header_t.checksum= (uint8_t) 0;
  memcpy(ipv4_header_t.src_ip, layer->addr, sizeof(ipv4_addr_t)); 
  memcpy(ipv4_header_t.dest_ip,dst, sizeof(ipv4_addr_t) );
  memcpy(ipv4_header_t.payload, payload, payload_len);
  //Calculo de checksum:
  ipv4_header_t.checksum = ipv4_checksum( (unsigned char *) &ipv4_header_t, IPV4_HDR_LEN); // IPV4_HDR_LEN defined in ipv4.h 
  // 1500 ETH - 20 cab IP = 1480
  //ipv4_open ya lo hace el cliente o el servidor.
  //for knowing MAC address of dest, we call arp_resolve function from arp.c & arp.h
  //2 cases: if dst is in my same subnet: 
  //Usamos el parametro "layer" para ver nuestra tambla de rutas (se creara en el cliente y servidor respectivamente, pero no aqui).
  //Case 1: dst is in my same subnet:
  //MAYBE: I want to create an auxiliary route (mine) so we can put it as a parameter in "ipv4_route_lookup"
  //if not, ¿why do we want "layer" as a parameter?
  ipv4_route_t * route_to_dst =  ipv4_route_table_lookup ( layer->routing_table, dst);
  if(route_to_dst == NULL){
    printf("Error: Ruta no accesible.");
  }



  /*
  ipv4_addr_t mysubnet = {0,0,0,0};//empty ipv4 subnet direction.
  for(int i = 0; i < IPv4_ADDR_SIZE; i++  ){
    mysubnet[i] = ipv4_header_t.src_ip[i] & dst[i];//Create subnet
  }
  ipv4_route_t * my_route = ipv4_route_create( mysubnet, layer-> netmask, layer->iface, ipv4_header_t.src_ip);
  //Now that we have a route, we will call 
  
  if(ipv4_route_lookup ( my_route, dst) >= 0){
    char* my_subnet_str = (char*) malloc(sizeof(ipv4_addr_t));
    ipv4_addr_str( mysubnet, my_subnet_str);
    char* dest_ipv4_str = (char*) malloc(sizeof(ipv4_addr_t));
    ipv4_addr_str( dst, dest_ipv4_str);
    printf("dest_ip : %s is inside sender subnet, with address: %s and prefix_length: %d",my_subnet_str,dest_ipv4_str, prefix_length);
    free(my_subnet_str);
    free(dest_ipv4_str);
  }
  */

  return 0;
}

//1º rellenamos, 2º miramos siguiente salto para saber la IP destino, y luego haremos arp_resolve para saber la MAC.
int ipv4_recv(ipv4_layer_t * layer, uint8_t protocol,unsigned char buffer [], ipv4_addr_t sender, int buf_len,long int timeout)
{
  int payload_len;
  /* Comprobar parámetros */
  if (layer == NULL) {
    fprintf(stderr, "eth_recv(): ERROR: iface == NULL\n");

    return -1;
  }

  return 0;
}
