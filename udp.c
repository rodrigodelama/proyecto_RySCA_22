#include "udp.h"

//UDP datagram: 1472 bytes (1480bytes-8bytes from ip field)
struct udp_header {
    uint16_t src_port; //16 bits for Source Port Number.
    uint16_t dest_port; //16 bits for Destination Port Number.
    uint16_t datagram_length; //16 bits for Length.
    uint16_t checksum; //returned value from checksum() function. Other 16 bits for Checksum.
    //pseudocabecera ciertos parametros para calcular el checksum en basea a eso, mirar rfc
    //por default lo pondremos en 0
    //no comprobamos checksum en salida ni entrada
};

checksum = 0;

//open connection
int udp_open(ipv4_addr_t dest, int port)
{
    // Creamos nuestro Socket 
            


    // debemos hacer nuestro propio socket
    // unir una ip con un puerto

    return 0;
}

int udp_close(socket)
{

    return 0;
}

//send datagram
int udp_send(ipv4_addr_t dest, unsigned char * payload, int payload_len)
{

    return 0; //if all is well
}

int udp_recv(datagram)
{

    return 0;
}
