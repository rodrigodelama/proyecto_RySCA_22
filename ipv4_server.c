#include "global_dependencies.h"

#include "ipv4.h"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <origin_ip>\n");
        //printf("     <origin_ip>: Direccion ip de origen \n");
        printf("     <log_level>: Nivel superior de logs a usar\n");
        exit(-1);
    }

    switch(argv[1][0])
    { //todas las opciones empiezan por letra distinta, solo miro la primera
		case 't':
			log_set_level(LOG_TRACE);
			break;
		case 'd':
			log_set_level(LOG_DEBUG);
			break;
        default:
			log_set_level(LOG_INFO);
			break;
	}

    ipv4_layer_t* my_ip_iface = ipv4_open("./ipv4_config_server.txt", "./ipv4_route_table_server.txt");
    if(my_ip_iface == NULL){
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    unsigned char buffer[1460];
    long int timeout = -1;//Numero negativo, temporizador infinito. (si hacemos esto, quitamos el if, porque bytes_crvd sera positivo siempre (si ha habido un error sera negativo.))
    int bytes_rcvd = 0;
    while(1){
        bytes_rcvd = 0;
        ipv4_addr_t received_ip;
        bytes_rcvd = ipv4_recv(my_ip_iface, 17, buffer, received_ip, 0, timeout);
        log_debug("Bytes received -> %d\n",bytes_rcvd);
        if(bytes_rcvd > 0){//Si he recibido un paquete sin que haya habido un error ni se haya agotado el temporizador.
            ipv4_send(my_ip_iface, received_ip, 17, buffer,(bytes_rcvd - 20));
            break;//Salimos del bucle.
        }
    }

/*  MAYBE WE MUST SEND SOMETHING BACK FROM THE SERVER TO THE CLIENT ??  
    unsigned char fake_payload[1460];

    struct ipv4_header ipv4_header_t;
    memset(&ipv4_header_t, 0, sizeof(struct ipv4_header)); //Relleno la zona de memoria que guarda nuestra cabecera IP con 0s
    // Rellenamos sus campos
    ipv4_header_t.version_and_length = (uint8_t) VERSION_AND_LENGTH; //"dos campos de 4bytes" rellenado a mano en Hex
    ipv4_header_t.service_type = 0;
    ipv4_header_t.total_length = (uint16_t) IPV4_HDR_LEN + sizeof(fake_payload); //total length is hdr + data
    ipv4_header_t.identification = (uint16_t) ID;
    ipv4_header_t.frag_flags = (uint16_t) 0;
    ipv4_header_t.ttl = (uint8_t) TTL_DEF;
    ipv4_header_t.protocol = (uint8_t) 17; //UDP
    ipv4_header_t.checksum = (uint8_t) 0; //initally at 0
    memcpy(ipv4_header_t.src_ip, my_ip_iface->addr, sizeof(ipv4_addr_t));
    memcpy(ipv4_header_t.dest_ip, src_ip, sizeof(ipv4_addr_t));
    memcpy(ipv4_header_t.payload, fake_payload, sizeof(fake_payload));
    //Calculo de checksum:
    ipv4_header_t.checksum = ipv4_checksum( (unsigned char *) &ipv4_header_t, IPV4_HDR_LEN); // IPV4_HDR_LEN defined in ipv4.h 
                                                            // 1500 ETH - 20 cab IP = 1480
    ipv4_send(my_ip_iface, ipv4_header_t.dest_ip, ipv4_header_t.protocol, ipv4_header_t.payload, ipv4_header_t.total_length);
*/
    ipv4_close(my_ip_iface);
    return 0;
}
