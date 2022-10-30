#include "global_dependencies.h"
#include "udp.h"

//Client will send UDP messages to a determined ip add, to a certain port there

int main ( int argc, char * argv[] )
{   
    printf("argc = %d\n",argc);
    if(argc > 3 || argc == 2)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <listening_port> <log_level>\n");
        printf("     <listening_port>: Puerto en el que escuchará el sever\n");
        printf("     <log_level>: Nivel superior de logs a usar \n");
        exit(-1);
    }
    if (argc == 3)
    {
        switch(argv[2][0])
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
    }
    
    if(atoi(argv[1]) == 0 || atoi(argv[1]) < 0)
    {
        fprintf(stderr, "%s\n", "Puerto de escucha pasado como parámetro no es válido\n");
        exit(-1);
    }
    uint16_t listen_port = (u_int16_t) atoi(argv[1]);
    udp_layer_t * my_udp_layer = udp_open(listen_port, "./ipv4_config_server.txt", "./ipv4_route_table_server.txt");
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    unsigned char fake_payload[1446];
    long int timeout = -1; //Numero negativo, temporizador infinito. (si hacemos esto, quitamos el if, porque bytes_crvd sera positivo siempre (si ha habido un error sera negativo.))
    int bytes_rcvd = 0;

    while(1)
    {
        bytes_rcvd = 0;
        ipv4_addr_t received_ip;
        int received_port = 0;
        bytes_rcvd = udp_rcv(my_udp_layer,received_ip, received_port, fake_payload, 0, timeout);
        log_debug("Dest_port -> %d\n", received_port);
        log_debug("Bytes received -> %d\n", bytes_rcvd);
        if(bytes_rcvd > 0) //Si he recibido un paquete sin que haya habido un error ni se haya agotado el temporizador.
        {
            udp_send(my_udp_layer, received_ip, received_port, fake_payload, bytes_rcvd);
            break; //Salimos del bucle.
        }
    }
    //log_debug("Bytes of data sent by UDP send -> %d\n",bytes_sent);
    
    return 0;
}