#include "global_dependencies.h"

#include "udp.h"

#define PORT_NUM_SIZE 4

ipv4_addr_t RIPv2_ADDR_UDP = { 224, 0, 0, 9 };

struct udp_layer //our "socket"
{
    ipv4_layer_t* local_ip_stack;
    uint16_t local_port;
};

//UDP datagram
typedef struct udp_header
{
    uint16_t src_port; //16 bits for Source Port Number.
    uint16_t dest_port; //16 bits for Destination Port Number.
    uint16_t datagram_length; //16 bits for Length.
    uint16_t checksum; //returned value from checksum() function. Other 16 bits for Checksum.
    //pseudocabecera ciertos parametros para calcular el checksum en basea a eso, mirar rfc
    //por default lo pondremos en 0
    //no comprobamos checksum en salida ni entrada
    unsigned char payload[1452]; // MTU 1500 - 2Ocab eth - 20cab IP - 8cab UDP -> 1452.
} udp_header_t;

//open connection
udp_layer_t* udp_open(int src_port, char *file_conf, char *file_conf_route)
{
    // Creamos nuestro 'socket'
    udp_layer_t* my_udp_iface = (udp_layer_t*) malloc(sizeof(udp_layer_t));
    if (my_udp_iface == NULL)
    {
        fprintf(stderr, "udp_open(): ERROR en malloc()\n");
        return NULL;
    }
    my_udp_iface->local_ip_stack = ipv4_open(file_conf, file_conf_route);
    #ifdef DEBUG
        if(my_udp_iface->local_ip_stack == NULL){
            log_trace("Ipv4 interface has not been created correctly\n");
        }
    #endif
    my_udp_iface->local_port = src_port;
        log_debug("Source port -> %d \n",my_udp_iface->local_port);//Asignarse se asigna bien (dentro de udp_open)

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
int udp_send(udp_layer_t *my_udp_iface, ipv4_addr_t dest, uint16_t dest_port, unsigned char *payload, int payload_len)//payload_len -> tamaño campo de datos.
{
    // Campo datagram_length es cabecera UDP (8 bytes) + Campo de Datos (payload_len)
        log_debug("Payload_lenght passed as parameter -> %d\n", payload_len);
    
    // if (memcmp(dest, RIPv2_ADDR_UDP, sizeof(ipv4_addr_t)) == 0)
    // {
    //     //RIP
    // }

    if (my_udp_iface == NULL)
    {
        fprintf(stderr, "udp_send(): ERROR: udp_layer == NULL\n");
        return -1;
    }
    if(payload_len > 1452){
        fprintf(stderr, "%s\n", "Error: Tamaño de datos demasiado grande (Límite = 1452 bytes)...\n");
        exit(-1);
    }
    //rellenar datagrama
    udp_header_t udp_header_t;
    memset(&udp_header_t, 0, sizeof(udp_header_t)); //Relleno la zona de memoria que guarda nuestra cabecera IP con 0s
    	log_debug("Source port before htons -> %d \n",my_udp_iface->local_port);
    udp_header_t.src_port = htons(my_udp_iface->local_port);
    	log_debug("Source port after htons -> %d \n",udp_header_t.src_port);

    	log_debug("Dest port before htons -> %d \n",dest_port);
    udp_header_t.dest_port = htons(dest_port);
    	log_debug("Dest port after htons -> %d \n",udp_header_t.dest_port);

    //memcpy(payload, udp_header_t.payload, payload_len); //copying char arrays (somos tonticos, copiabamos lo vacio en lo no-vacio)

    udp_header_t.datagram_length = htons( 8 + payload_len);
    	log_debug("Datagram_length before htons -> %d \n",ntohs(udp_header_t.datagram_length));
    	log_debug("Payload_len after htons -> %d \n",udp_header_t.datagram_length);
    udp_header_t.checksum = 0; //initially 0, maybe a later improvement
    memcpy(udp_header_t.payload, payload, payload_len);
    	log_debug("Payload -> %s \n",udp_header_t.payload);
    	log_debug("udp_header_t.payload -> %d\n", udp_header_t.payload);
    int bytes_sent = ipv4_send(my_udp_iface->local_ip_stack, dest, UDP_PROTOCOL_TYPE, (unsigned char *) &udp_header_t, (payload_len + 8)); //No estamos mandando el paquete UDP, estabamos mandando la payload de UDP.
    	log_trace("UDP datagram sent. Number of data bytes sent -> %d\n", bytes_sent - 8 );
    if(bytes_sent == -1)
    {
        fprintf(stderr, "udp_send(): ERROR: ipv4_send failed\n");
        return -1;
    }
    bytes_sent = bytes_sent - 8;//ipv4_send returns bytes sent inside ipv4 data (payload) field.
    //Here, we will return the size of the data field from udp sent apart from the header (8bytes).
    return bytes_sent; //if all is well
}

int udp_rcv(udp_layer_t *my_udp_layer,ipv4_addr_t src, uint16_t* dest_port, unsigned char buffer[], int buf_len, long int timeout)
{
	int payload_len;

	/* Comprobar parámetros */
	if (my_udp_layer == NULL)
	{
		fprintf(stderr, "udp_recv(): ERROR: my_udp_layer == NULL\n");
		return -1;
	}

	/* Inicializar temporizador para mantener timeout si se reciben tramas con
		tipo incorrecto. */
	timerms_t timer;
	timerms_reset(&timer, timeout);

	int datagram_len = 0;
	//UDP HEADER_SIZE
	int udp_buf_len = UDP_HEADER_SIZE + buf_len;//ipv4_send(), sends hader (8bytes) + payload (buf_len).
	unsigned char udp_buffer[udp_buf_len];
	udp_header_t * udp_datagram_ptr = NULL;
	int is_dest_port;//To check if the dest port of  the recieved datagram is my local port.

	do {
		long int time_left = timerms_left(&timer);

		/* Recibir trama del interfaz Ethernet y procesar errores */
		datagram_len = ipv4_recv(my_udp_layer->local_ip_stack, UDP_PROTOCOL_TYPE, udp_buffer, src, udp_buf_len, time_left);
		if (datagram_len < 0)
		{
			printf("udp_recv(): ERROR en ipv4_recv()");
			return -1;
		} else if (datagram_len == 0) {
			/* Timeout! */
			fprintf(stderr, "udp_rcv(): timeout in ipv4_recv()\n\n");
			return 0;
		} else if (datagram_len < UDP_HEADER_SIZE) {
			fprintf(stderr, "udp_recv(): Datagrama de tamaño invalido: %d bytes\n",
					datagram_len);
			continue; //TODO: pq hay un "continue" aqui??
		}

		/* Comprobar si es la trama que estamos buscando */
		udp_datagram_ptr = (udp_header_t *) udp_buffer;
		is_dest_port = (ntohs(udp_datagram_ptr->dest_port) == my_udp_layer->local_port);
		//is_src_port = (ntohs(udp_datagram_ptr->src_port) == dest_port); -> dest_port parametro de salida.
	} while (! is_dest_port);
	
	/* Trama recibida con 'tipo' indicado. Copiar datos y dirección MAC origen */
	
	payload_len = datagram_len - UDP_HEADER_SIZE;
	if (buf_len > payload_len)
	{
		buf_len = payload_len;
	}
	memcpy(buffer, udp_datagram_ptr->payload, buf_len);
	// dest_port = ntohs(udp_datagram_ptr->src_port);
	*(dest_port) = ntohs(udp_datagram_ptr->src_port);
	// uint16_t* dest_port_returned;
	// *(dest_port_returned) = ntohs(udp_datagram_ptr->src_port);
	// memcpy(dest_port, dest_port_returned, sizeof(udp_datagram_ptr->dest_port));
	//payload_len = payload_len - UDP_HEADER_SIZE;//ya lo hacemos mas arriba, sino le restamos 16 en vez de 8 
		log_debug("UDP bytes received -> %d\n", payload_len);

	return payload_len;
}

int random_port_generator(void) //Funcion de aula global.
{
    /* Inicializar semilla para rand() */
    unsigned int seed = time(NULL);
    srand(seed);
        /* Generar número aleatorio entre 0 y RAND_MAX */
        int dice = rand();
        /* Número entero aleatorio entre 1 y 10 */
        dice = 1024 + (int) (65535.0 * dice / (RAND_MAX + 1.0));
            log_debug("Random port -> %i\n", dice);

    return dice;
}
