#include "global_dependencies.h"

#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "arp.h"

//Cuando capturemos ip, hacerlo en el que envía la trama, dado que en lightning descarta las tramas con el checksum mal.   
/* Dirección IPv4 a cero: "0.0.0.0" */
ipv4_addr_t IPv4_ZERO_ADDR = { 0, 0, 0, 0 };

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
  if (str != NULL)
  {
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

  if (str != NULL)
  {
    unsigned int addr_int[IPv4_ADDR_SIZE];
    int len = sscanf(str, "%d.%d.%d.%d", 
                    &addr_int[0], &addr_int[1], 
                    &addr_int[2], &addr_int[3]);

    if (len == IPv4_ADDR_SIZE)
    {
      int i;
      for (i = 0; i < IPv4_ADDR_SIZE; i++)
      {
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
  for (i = 0; i < len; i = i+2)
  {
    word16 = ((data[i] << 8) & 0xFF00) + (data[i+1] & 0x00FF);
    sum = sum + (unsigned int) word16;	
  }

  /* Take only 16 bits out of the 32 bit sum and add up the carries */
  while (sum >> 16)
  {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /* One's complement the result */
  sum = ~sum;

  return (uint16_t) sum;
}

ipv4_layer_t* ipv4_open(char * file_conf, char * file_conf_route)
{

  /* 1. Crear layer -> routing_table */
  ipv4_layer_t *ipv4_layer = (ipv4_layer_t*) malloc(sizeof(ipv4_layer_t)); //allocate memory
  memset(&ipv4_layer, 0, sizeof(ipv4_layer_t));
  if (ipv4_layer == NULL)
  {
    fprintf(stderr, "ipv4_open(): ERROR en malloc()\n");
    return NULL;
  }
  
  char iface_name[32]; //eth hard limit on iface length
  /* 2. Leer direcciones y subred de file_conf */
  if (ipv4_config_read(file_conf, iface_name, ipv4_layer->addr, ipv4_layer->netmask) != 0)
  {
                                //contains "layer" as in iface, addr, and netmask
    fprintf(stderr,"ERROR: file could not be opened correctly.\n");
    exit(-1);
  }
    /*La función devuelve '0' si el fichero de configuración se ha leido correctamente.*/
  /* 3. Leer tabla de reenvío IP de file_conf_route */
  if(ipv4_route_table_read(file_conf_route, ipv4_layer->routing_table) != 0)
  {
    fprintf(stderr,"ERROR: file could not be opened correctly.\n");
    exit(-1);
  }
  /* 4. Inicializar capa Ethernet con eth_open() */
  //Guardamos el manejador en el campo de "iface".
  ipv4_layer->iface = eth_open(iface_name); //Returns eth interface controller  

  return ipv4_layer;
}

//inspired on eth_close
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
                //layer = manejador ipv4.
int ipv4_send (ipv4_layer_t * layer, ipv4_addr_t dst, uint8_t protocol, unsigned char * payload, int payload_len)
{
  /* Comprobar parámetros */
  if (layer == NULL)
  {
    fprintf(stderr, "ipv4_send(): ERROR: ipv4_layer == NULL\n");
    return -1;
  }
  //maybe we might need to check other parameters

  /* Crear el paquete IPv4 */
  struct ipv4_header ipv4_header_t;
  memset(&ipv4_header_t, 0, sizeof(struct ipv4_header)); //Relleno la zona de memoria que guarda nuestra cabecera IP con 0s
  /* Rellenamos sus campos */
  ipv4_header_t.version_and_length = (uint8_t) VERSION_AND_LENGTH; //"dos campos de 4bytes" rellenado a mano en Hex
  ipv4_header_t.service_type = 0;
  ipv4_header_t.total_length = htons((uint16_t) payload_len);
  ipv4_header_t.identification = htons((uint16_t) ID);
  ipv4_header_t.frag_flags = (uint16_t) 0;
  ipv4_header_t.ttl = (uint8_t) TTL_DEF;
  ipv4_header_t.protocol = (uint8_t) protocol; //passed as parameter.
  ipv4_header_t.checksum = (uint8_t) 0; //initally at 0
  memcpy(ipv4_header_t.src_ip, layer->addr, sizeof(ipv4_addr_t)); 
  memcpy(ipv4_header_t.dest_ip, dst, sizeof(ipv4_addr_t));
  memcpy(ipv4_header_t.payload, payload, payload_len);
  //Calculo de checksum:
  ipv4_header_t.checksum = htons(ipv4_checksum( (unsigned char *) &ipv4_header_t, IPV4_HDR_LEN)); // IPV4_HDR_LEN defined in ipv4.h 
  // 1500 ETH - 20 cab IP = 1480
  //ipv4_open ya lo hace el cliente o el servidor.
  //for knowing MAC address of dest, we call arp_resolve function from arp.c & arp.h
  //2 cases: if dst is in my same subnet: 
  //Usamos el parametro "layer" para ver nuestra tambla de rutas (se creara en el cliente y servidor respectivamente, pero no aqui).
  //Case 1: dst is in my same subnet:
  //MAYBE: I want to create an auxiliary route (mine) so we can put it as a parameter in "ipv4_route_lookup"
  //if not, ¿why do we want "layer" as a parameter?
  ipv4_route_t *route_to_dst =  ipv4_route_table_lookup (layer->routing_table, dst); //returns most efficient route
  if(route_to_dst == NULL)
  {
    printf("Error: Ruta no accesible o no ha sido posible realizar la búsqueda.\n");
    return -1;
  }

  mac_addr_t mac_dest; //mac de ip_dest si misma subred, sino mac de gateway_addr.
  eth_iface_t* sender_iface = eth_open (route_to_dst -> iface);

  int bytes_sent = 0;
  int err_arp = 0;
  if(route_to_dst->gateway_addr == 0) //El destino está en nuestra subred.
  {
    err_arp = arp_resolve(sender_iface, dst, mac_dest); //mac destino
    if(err_arp != 0)
    {
      printf("Error: function arp_resolve not working...\n");
      return -1;
    }
    printf("Sendig inside our subnet....\n");
    bytes_sent = eth_send (sender_iface, mac_dest, PROT_TYPE_IPV4, (unsigned char *) &ipv4_header_t, sizeof(struct ipv4_header));
    if(bytes_sent == -1)
    {
      printf("Error sending eth frame....");
      return -1;
    }
  } else { //Fuera de nuestra subred, pedimos mac a la ip_gateway
    err_arp = arp_resolve(sender_iface, route_to_dst->gateway_addr, mac_dest); //mac gateway
    if(err_arp != 0 )
    {
      printf("Error: function arp_resolve not working...\n");
      return -1;
    }
    printf("Sendig outside our subnet....\n");
    bytes_sent = eth_send (sender_iface, mac_dest, PROT_TYPE_IPV4, (unsigned char *) &ipv4_header_t, sizeof(struct ipv4_header));
    if(bytes_sent == -1)
    {
      printf("Error sending eth frame....\n");
      return -1;
    }
  }

  //IPV4_HDR_LEN inside eth.h.  
  //bytes_sent is what eth sends, minus 20 of eth header - ipv4 hdr length
  return (bytes_sent - 20 - IPV4_HDR_LEN); //eth header size inside eth.c, not included.
}

//1º rellenamos, 2º miramos siguiente salto para saber la IP destino, y luego haremos arp_resolve para saber la MAC.
//input: layer, protocol, buf_len, timeout
// output: buffer, sender 
int ipv4_recv(ipv4_layer_t *layer, uint8_t protocol, unsigned char buffer[], ipv4_addr_t sender, int buf_len, long int timeout)
{
  int payload_len; 
  /* Comprobar parámetros */
  if(layer == NULL)
  {
    fprintf(stderr, "ipv4_recv(): ERROR: layer == NULL\n");
    return -1;
  }
  /* Inicializar temporizador para mantener timeout si se reciben tramas con tipo incorrecto. */
  timerms_t timer;
  timerms_reset(&timer, timeout);

  int packet_len = 0;
  int packet_buf_len = IPV4_HDR_LEN + buf_len;
  unsigned char ipv4_buffer[packet_buf_len];
  struct ipv4_header *ipv4_packet_ptr = NULL;
  int is_target_type;
  int is_my_ip;
  mac_addr_t mac_src;
  int original_checksum;
  int is_my_checksum = 0; //declared as false initially

  do
  {
    long int time_left = timerms_left(&timer);

    /* Recibir trama del interfaz Ethernet y procesar errores */
    packet_len = eth_recv (layer->iface, mac_src, PROT_TYPE_IPV4, ipv4_buffer, packet_buf_len, time_left);
    if(packet_len < 0)
    {
      fprintf(stderr, "ipv4_recv(): ERROR en eth_recv()");
      return -1;
    } else if (packet_len == 0) {
      /* Timeout! */
      return 0;
    } else if (packet_len < IPV4_HDR_LEN) {//Minimum packet length = ipv4_header (20 bytes) + 0 bytes payload.
      fprintf(stderr, "ipv4_recv(): cabecera incorrecta, paquete incompleto: %d bytes\n", packet_len);
      continue;
    } 

    /* Comprobar si es el paquete que estamos buscando */
    ipv4_packet_ptr = (struct ipv4_header *) ipv4_buffer;
    is_my_ip = (memcmp(ipv4_packet_ptr->dest_ip, layer->addr, IPv4_ADDR_SIZE) == 0); //comparing memory reults in a 1 if true
    is_target_type = (ntohs(ipv4_packet_ptr->protocol) == protocol);

    original_checksum = ntohs(ipv4_packet_ptr->checksum);
    ipv4_packet_ptr->checksum = 0;
    uint16_t calculated_checksum = ipv4_checksum ((unsigned char *) ipv4_buffer, packet_buf_len);
    if (original_checksum == calculated_checksum)
    {
      is_my_checksum = 1; //is true
    } //by default, has value 1 (False).
  } while ( !(is_my_ip && is_target_type && is_my_checksum)); //if all is 1, !1 = 0, therefore the do-while will end
  
  /* Paquete recibido con 'protocolo' indicado. Copiar datos y dirección IP origen */
  memcpy(sender, ipv4_packet_ptr->src_ip, IPv4_ADDR_SIZE);
  payload_len = packet_len - IPV4_HDR_LEN;
  if(buf_len > payload_len)
  {
    buf_len = payload_len; //we adjust the size if buffer is bigger
  }
  memcpy(buffer, ipv4_packet_ptr->payload, buf_len);

  return payload_len;
}
