#include "global_dependencies.h"

#include "udp.h"

struct udp_layer //our "socket"
{
    ipv4_layer_t* local_ip_stack;
    uint16_t local_port;
};

//UDP datagram
struct udp_header
{
    uint16_t src_port; //16 bits for Source Port Number.
    uint16_t dest_port; //16 bits for Destination Port Number.
    uint16_t datagram_length; //16 bits for Length.
    uint16_t checksum; //returned value from checksum() function. Other 16 bits for Checksum.
    //pseudocabecera ciertos parametros para calcular el checksum en basea a eso, mirar rfc
    //por default lo pondremos en 0
    //no comprobamos checksum en salida ni entrada
    unsigned char payload[1472]; // 1472 bytes (1480bytes-8bytes from ip field)
};

//open connection
udp_layer_t* udp_open(ipv4_addr_t src, int src_port, char *file_conf, char *file_conf_route)
{
    // Creamos nuestro 'socket'
    udp_layer_t* my_udp_iface = (udp_layer_t*) malloc(sizeof(udp_layer_t));
    if (my_udp_iface == NULL)
    {
        fprintf(stderr, "udp_open(): ERROR en malloc()\n");
        return NULL;
    }
    my_udp_iface->local_ip_stack = ipv4_open(file_conf, file_conf_route);
    my_udp_iface->local_port = src_port;

    return my_udp_iface;
}

int udp_close(udp_layer_t* my_udp_iface)
{
    int err = -1;

    if (my_udp_iface != NULL)
    {
        err = ipv4_close(my_udp_iface->local_ip_stack);
        free(my_udp_iface);
    }
    return err;
}

//send datagram
int udp_send(udp_layer_t *my_udp_iface, ipv4_addr_t dest, uint16_t dest_port, unsigned char *payload, int payload_len)
{
    if (my_udp_iface == NULL)
    {
        fprintf(stderr, "udp_send(): ERROR: udp_layer == NULL\n");
        return -1;
    }
    //rellenar datagrama
    struct udp_header udp_header_t;
      memset(&udp_header_t, 0, sizeof(struct udp_header)); //Relleno la zona de memoria que guarda nuestra cabecera IP con 0s
    udp_header_t.src_port = my_udp_iface->local_port;
    udp_header_t.dest_port = dest_port;
    memcpy(payload, udp_header_t.payload, payload_len); //copying char arrays
    udp_header_t.datagram_length = payload_len;
    udp_header_t.checksum = 0; //initially 0, maybe a later improvement
    int bytes_sent = ipv4_send(my_udp_iface->local_ip_stack, dest, UDP_PROTOCOL_TYPE, payload, (payload_len + 8));
    if(bytes_sent == -1)
    {
        fprintf(stderr, "udp_send(): ERROR: ipv4_send failed\n");
        return -1;
    }
    return 0; //if all is well
}

int udp_rcv(udp_layer_t *my_udp_layer, ipv4_addr_t src, uint16_t src_port, uint16_t dest_port, unsigned char *payload, int payload_len, int buf_len, long int timeout)
{

    return 0;
}
