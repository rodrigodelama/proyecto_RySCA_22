#include "global_dependencies.h"

#include "ipv4.h"

int main(int argc, char* argv[])
{
    if(argc > 2)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <log_level>: Nivel superior de logs a usar\n");
        exit(-1);
    }

    if(argc == 2)
    {
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
    } else {
        log_set_level(LOG_INFO);
    }

    ipv4_layer_t* my_ip_iface = ipv4_open("./ipv4_config_server.txt", "./ipv4_route_table_server.txt");
    if(my_ip_iface == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    unsigned char buffer[1460];
    long int timeout = -1; //Numero negativo, temporizador infinito. (si hacemos esto, quitamos el if, porque bytes_crvd sera positivo siempre (si ha habido un error sera negativo.))
    int bytes_rcvd = 0;
    while(1)
    {
        bytes_rcvd = 0;
        ipv4_addr_t received_ip;
        bytes_rcvd = ipv4_recv(my_ip_iface, 17, buffer, received_ip, 0, timeout);
        log_debug("Bytes received -> %d\n", bytes_rcvd);
        if(bytes_rcvd > 0) //Si he recibido un paquete sin que haya habido un error ni se haya agotado el temporizador.
        {
            ipv4_send(my_ip_iface, received_ip, 17, buffer, (bytes_rcvd - 20));
            //FIXME:break; //Salimos del bucle.
        }
    }
    ipv4_close(my_ip_iface);
    return 0;
}
