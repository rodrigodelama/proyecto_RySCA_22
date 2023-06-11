#include "global_dependencies.h"
#include "udp.h"

//Client will send UDP messages to a determined ip add, to a certain port there

int main ( int argc, char * argv[] )
{   
    if(argc > 4 || argc == 1 || argc < 3)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <target_ip> <target_port> <log_level>\n");
        printf("     <target_ip>: Direccion ip de destino \n");
        printf("     <target_port>: Puerto de destino al que mandar.\n");
        printf("     <log_level>: Nivel superior de logs a usar \n");
        exit(-1);
    }

    if(argc == 4)
    {
        switch(argv[3][0])
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
    } else {
        log_set_level(LOG_INFO);
    }

    ipv4_addr_t dest_ip;

    if(ipv4_str_addr(argv[1], dest_ip) == -1)
    {
        fprintf(stderr, "%s\n", "IP pasada como parámetro no válida\n");
        exit(-1);
    }
    if(atoi(argv[2]) == 0 || atoi(argv[2]) < 0){
        fprintf(stderr, "%s\n", "Puerto de destino pasado como parámetro no es válido\n");
        exit(-1);
    }
    uint16_t destport = (u_int16_t) atoi(argv[2]);
    udp_layer_t * my_udp_layer = udp_open(random_port_generator(), "./ipv4_config_client.txt", "./ipv4_route_table_client.txt");
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    unsigned char fake_payload[1200];
    int bytes_sent = udp_send(my_udp_layer, dest_ip, destport, fake_payload, 0);//Solamente queremos que mande ahora mismo la cabecera udp.
        log_debug("Bytes of data sent by UDP send -> %d\n",bytes_sent);
    unsigned char fake_payload_rcv[1200];
    int timeout = 6000;
    int bytes_rcvd = udp_rcv(my_udp_layer,dest_ip, &destport, fake_payload_rcv, 0, timeout);
    if(bytes_rcvd == 0)
    {
        log_trace("Reception timeout reached...\n\n");
    } else {
        log_trace("ECHO Packet received!!\n");
    }
    udp_close(my_udp_layer);
    return 0;
}

