#include "global_dependencies.h"

#include "ripv2.h"
#include "ripv2_route_table.h"

ipv4_addr_t IPv4_ZERO_ADDR_3 = { 0, 0, 0, 0 };
ipv4_addr_t RIPv2_ADDR_2 = { 224, 0, 0, 9 };

long int least_time(ripv2_route_table_t * rip_table)
{
    long int time_left;
    long int min_time = 180000;
    ripv2_route_t * current_route = NULL;

    for(int i = 0; i < RIPv2_ROUTE_TABLE_SIZE; i++)
    {
        current_route = ripv2_route_table_get(rip_table, i);
        if (current_route != NULL)
        {
            time_left = timerms_left(&current_route->timer_ripv2);
                log_trace("the timer %d has left: %ld\n", i, time_left);
            if(time_left < min_time)
            {
                min_time = time_left;
            }
        }
    }
    return min_time;
}

int index_least_time(ripv2_route_table_t * rip_table)
{
    int index;
    long int time_left;
    long int min_time = 180000;
    ripv2_route_t * current_route = NULL;

    for(int i = 0; i < RIPv2_ROUTE_TABLE_SIZE; i++) //itera hasta ripv2routetable size
    {
    current_route = ripv2_route_table_get(rip_table, i);
        if(current_route != NULL)
        {
            time_left = timerms_left(&current_route->timer_ripv2);

            if(time_left < min_time)
            {
                min_time = time_left;
                index = i;
            }
        }
    }
    return index;
}

int set_gateway(ipv4_addr_t next_hop)
{
    if(memcmp(IPv4_ZERO_ADDR_3, next_hop, IPv4_ADDR_SIZE) == 0) //memcmp is 0 is equal
    {
        // si sale 0, la gateway ha de ser la IP de donde lo hemos recibido
        return 0;
    }
    // si no es igual a "0.0.0.0" , la gateway ha de ser next_hop
    return 1;
}

int number_of_routes(ripv2_route_table_t * rip_table)
{
    ripv2_route_t * route = NULL;
    int counter = 0;
    for(int i = 0; i < RIPv2_ROUTE_TABLE_SIZE; i++)
    {
        route = ripv2_route_table_get(rip_table, i);
        if(route != NULL)
        {
            counter++;
        }
    }
    return counter;
}

int main ( int argc, char * argv[] )
{   
    if(argc > 3 || argc < 1)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <log_level>\n");
        printf("     <log_level>: Nivel superior de logs a usar \n");
        exit(-1);
    }

    if(argc == 2)
    {
        switch(argv[1][0])
        { //opciones empiezan por letra distinta, miramos la primera
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

    ipv4_addr_t source_ip; // from where the recieved packet came
    uint16_t server_port = RIPv2_PORT;
    uint16_t client_port;
    unsigned char buffer_rip[LEN_PAYLOAD_RIP];
    
    udp_layer_t * my_udp_layer = udp_open(server_port, "./ipv4_config_server.txt", "./ipv4_route_table_server.txt");
        log_trace("udp_layer configuration DONE\n");
    
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }

    ripv2_route_table_t * rip_table = ripv2_route_table_create(); //Creamos la tabla de rutas
    ripv2_route_table_read ("./ripv2_route_table_server.txt", rip_table); //Rellenamos la tabla

    int index_min;
    int is_our_route;
    int route_index;
    ipv4_addr_t next_hop;
    int calc_gateway;
    char rip_iface[10] = "eth1";

    log_trace("Building (REQUEST) message\n");    
    ripv2_msg_t request_message;//Si no hago el malloc, me dice que la variable no esta inicializada ??
    memset(&request_message, 0, sizeof(ripv2_msg_t));
    //Cabecera RIP:
    request_message.type = (uint8_t) 1; //request
    request_message.version = (uint8_t) 2; //response
    request_message.dominio_encaminamiento = htons((uint16_t) 0x0000);
    //Entrada 1, vector distancia:
    request_message.vectores_distancia[0].familia_dirs = htons((uint16_t) 0x0000);
    //log_debug("Familia_dirs");
    request_message.vectores_distancia[0].etiqueta_ruta = htons((uint16_t) 0x0000);
    memcpy(request_message.vectores_distancia[0].subred , IPv4_ZERO_ADDR_3, sizeof(ipv4_addr_t));
    memcpy(request_message.vectores_distancia[0].subnet_mask , IPv4_ZERO_ADDR_3, sizeof(ipv4_addr_t));
    memcpy(request_message.vectores_distancia[0].next_hop, IPv4_ZERO_ADDR_3, sizeof(ipv4_addr_t));
    request_message.vectores_distancia[0].metric = htonl((uint32_t) 16);

    int length_request = RIPv2_MESSAGE_HEADER_SIZE + (RIPv2_DISTANCE_VECTOR_ENTRY_SIZE * 1);
        log_trace("Length of request packet -> %d\n", length_request);
    int bytes_sent = udp_send(my_udp_layer, RIPv2_ADDR_2, RIPv2_PORT, (unsigned char*) &request_message, length_request);
        log_trace("Bytes of data sent by UDP send -> %d\n", bytes_sent);

    while (1)
    {
        ripv2_route_table_print(rip_table);
        printf("\n");

        long int udp_timeout = least_time(rip_table);
            log_trace("udp_Timeout -> %ld\n", udp_timeout);

        int bytes_rcvd = udp_rcv(my_udp_layer, source_ip, &client_port, buffer_rip, sizeof(ripv2_msg_t), udp_timeout);
            log_trace("Total number of bytes received -> %d \n", bytes_rcvd);

        int numero_de_vectores_distancia = (bytes_rcvd - RIPv2_MESSAGE_HEADER_SIZE) / RIPv2_DISTANCE_VECTOR_ENTRY_SIZE ; //deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
            log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);
        
        ripv2_msg_t* ripv2_msg = (ripv2_msg_t*) buffer_rip;

        long int expiration_time = least_time(rip_table); // route with smallest timer found
        index_min = index_least_time(rip_table);
        
        ripv2_route_t* current_route = ripv2_route_table_get(rip_table, index_min);
        char subnet_str[IPv4_STR_MAX_LENGTH];
        
        ipv4_addr_str(current_route->subnet_addr, subnet_str);

        if(bytes_rcvd < 0) // should never happen
        {
            fprintf(stderr, "Error on recieved UDP datagram");
            return(-1);
        } else if (bytes_rcvd == 0) { // we will eliminate bca timer is up
            log_trace("Timer's up, route %s has been eliminated", subnet_str);
            ripv2_route_table_remove(rip_table, index_min);
            ripv2_route_table_print(rip_table);
        } 
    }
}
