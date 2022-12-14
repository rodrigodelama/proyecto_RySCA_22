#include "global_dependencies.h"
#include "ipv4_dependencies.h"

#include "ripv2.h"
#include "ripv2_route_table.h"
ipv4_addr_t IPv4_ZERO_ADDR_3 = { 0, 0, 0, 0 };
/*
Generar el response (toda la tabla):
Crear cabecera IPv4+UDP con:
ipv4.destino = 10.0.1.1 (IPv4 del router del request)
udp.puerto_destino = puerto origen del request / 520 (RIP port) para periodic responses
udp.puerto_origen = 520 (RIP port)
Rellenar las entradas necesarias en el mensaje RIPv2:
ripv2.command = 2 (response)
ripv2.version = 2 (RIPv2)
ripv2.dom_encam = 0x0000
ripv2.RTE[i].familia_de_direcciones = 2 (IP)
ripv2.RTE[i].dirección= 10.0.30.0 (ejemplo)
ripv2.RTE[i].máscara = 255.255.255.0 (ejemplo)
ripv2.RTE[i].siguiente_salto = 0.0.0.0
ripv2.RTE[i].métrica = 1

32 bits métrica -> hacer htonl()
*/

int least_time(ripv2_route_table_t * rip_table)
{
    long int time_left;
    long int min_time = 180000;
    ripv2_route_t * current_route= NULL;

    for(int i = 0; i < RIPv2_ROUTE_TABLE_SIZE; i++)
    {
        current_route = rip_route_table_get(rip_table, i);
    if (current_route != NULL)
    {
        time_left = timerms_left(&current_route->timer_ripv2);
        printf("the timer %d has left: %ld\n", i, time_left );
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

    for(int i = 0; i < RIPv2_ROUTE_TABLE_SIZE; i++) //itera hasta ripv2routetablesize, cuanto es este valor?????
    {
    current_route = rip_route_table_get(rip_table, i);
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
    } else {
        // si no es igual a "0.0.0.0" , la gateway ha de ser next_hop
        return 1;
    }
}

int number_of_routes(ripv2_route_table_t * rip_table)
{
    ripv2_route_t * route = NULL;
    int counter = 0;
    int i;
    for(i = 0; i < RIPv2_ROUTE_TABLE_SIZE;i++)
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
    if(argc > 4 || argc == 1 || argc < 3)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <target_ip> <log_level>\n");
        printf("     <target_ip>: Direccion ip de destino \n");//No target port, always 520.
        printf("     <log_level>: Nivel superior de logs a usar \n");
        exit(-1);
    }

    if(argc == 3)
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
    } else {
        log_set_level(LOG_INFO);
    }

    ipv4_addr_t dest_ip;
    uint16_t server_port = RIPv2_PORT;
    uint16_t client_port;
    unsigned char buffer_rip[LEN_PAYLOAD_RIP]; //? cuanto? 1400 maybe change later
    
    udp_layer_t * my_udp_layer = udp_open(server_port, "./ipv4_config_client.txt", "./ipv4_route_table_client.txt");
        log_trace("udp_layer configuration DONE\n");
    
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    ripv2_route_table_t * rip_table = ripv2_route_table_create(); //Creamos la tabla de rutas
    ripv2_route_table_read ("./ripv2_route_table_server.txt", rip_table); //Rellenamos la tabla
    ripv2_route_table_print(rip_table);

    int index_min;
    int is_our_route;
    int route_index;
    ipv4_addr_t* the_gw;
    int calc_gateway;
    char rip_iface[10] = "eth1";
    
    while (1)
    {
        long int timeout = least_time(rip_table);
        //En la primera iteración, tenemos tabla con cosas.
        int bytes_rcvd = udp_rcv(my_udp_layer, dest_ip, &client_port, buffer_rip, sizeof(ripv2_msg_t), timeout); //udp ya nos devuelve el número de bytes útiles (no worries en teoría). 
            log_debug("Total number of bytes received -> %d \n", bytes_rcvd);
        //SIEMPRE hay que mirar el origen del mensaje, si es mi padre, solamente actualizo la metrica y reseteo el timer. Si no viene de mi padre (mi garteway)
        //hay que mirar tambien si la métrica es mejor y si eso actualizar.

        int numero_de_vectores_distancia = (bytes_rcvd - RIPv2_MESSAGE_HEADER_SIZE) / RIPv2_DISTANCE_VECTOR_ENTRY_SIZE ; //deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
            log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);
        
        ripv2_msg_t* ripv2_msg = (ripv2_msg_t*) buffer_rip;
        // expire_time + index_min es sobre todo para encontrar la ruta que ha expirado, y poder seleccionarla con la funcion "ripv2_route_table_get()" 
        // y (llegado el caso) eliminarla con la funcion ripv2_route_table_remove(). -> es sobre todo para tenerla localizada.
        long int expire_time = least_time(rip_table); // route with smallest timer found
        index_min = index_least_time(rip_table); // route

        ripv2_route_t* current_route = ripv2_route_table_get(rip_table, index_min); //ruta con menor temporizador
        char subnet_str[IPv4_STR_MAX_LENGTH];
        ipv4_addr_str(current_route->subnet_addr, subnet_str);

        if(bytes_rcvd < 0) // should never happen
        {
            printf(stderr, "Error on recieved UDP datagram");
            return(-1);

        } else if(bytes_rcvd == 0) { // we will eliminate
            printf("Timer's up, route %s has been eliminated", subnet_str);
            ripv2_route_table_remove(rip_table, index_min);
            ripv2_route_table_print(rip_table);

        } else if(bytes_rcvd > 0 && expire_time == 0) { //Preguntar si hay que ver el caso de que expiren mas de una a la vez ¿para esto es expire_time? .
            for(int i = 0; i < numero_de_vectores_distancia; i++)
            {
                // int index_min;
                route_index = 0;
                route_index = ripv2_route_t_find(rip_table, ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask);
                if(index_min == route_index)
                {
                    is_our_route = 1;
                }
            }

            // this if might not be needed
            if(is_our_route != 1)
            {
                printf("Deleted the route: %s", subnet_str);
                is_our_route = 0;
                ripv2_route_table_remove(rip_table, index_min);
                ripv2_route_table_print(rip_table);
            }
        }

        // someone sends us their routes
        if(ripv2_msg->type == RIPv2_RESPONSE) // no hace falta hacer noths porque es un entero de 8bits (1 byte).
        {
            char str_route[IPv4_STR_MAX_LENGTH];
            printf("Received %d distance vectors", numero_de_vectores_distancia);
            for(int i = 0; i < numero_de_vectores_distancia; i++)
            {
                ipv4_addr_str(ripv2_msg->vectores_distancia[i].subred, str_route);

                printf("Route %d is %s\n ", i, str_route);

                route_index = rip_route_table_find(rip_table, ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask);

                // we dont have the route
                if(route_index == -1)
                {
                    printf("We dont have the route, we will create a new entry for it");
                    int metric_rcvd = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;

                    if(metric_rcvd < 16) //
                    {

                        the_gw = ripv2_msg->vectores_distancia[i].next_hop;
                        calc_gateway = set_gateway(ripv2_msg->vectores_distancia[i].next_hop);
                        if(calc_gateway == 0)
                        {
                            the_gw = dest_ip; // addr recieved as next hop is different from 0.0.0.0
                        }
                        
                        ripv2_route_t * new_route;
                        new_route = ripv2_route_create(ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask, rip_iface, the_gw, metric_rcvd);

                        rip_route_table_add(rip_table, new_route);
                        rip_route_table_print(rip_table);
                    } else {
                        printf("Distance for us is inf, route discarded"); //only for routes of 15 or less hops
                    }

                // we already have the route
                } else if(route_index > -1) {
                    printf("Recieved a route we already have, we will check if we must update it");

                    ripv2_route_t * ruta_de_nuestra_tabla = NULL;
                    ripv2_route_t * new_route = NULL;
                    ruta_de_nuestra_tabla = ripv2_route_table_get(rip_table, route_index);

                    if(memcmp(ripv2_msg->vectores_distancia[i].next_hop, ruta_de_nuestra_tabla->next_hop, IPv4_ADDR_SIZE) ==0)
                    {
                        if(ntohl(ripv2_msg->vectores_distancia[i].metric)+1 >= 16)
                        { //Si es del padre y 16 de metrica
                        rip_route_table_remove(rip_table,route_index);

                        } else if(ntohl(ripv2_msg->vectores_distancia[i].metric)+1 < 16) {

                        memcpy(new_route->subnet_addr, ruta_de_nuestra_tabla->subnet_addr, IPv4_ADDR_SIZE);
                        memcpy(new_route->subnet_mask, ruta_de_nuestra_tabla->subnet_mask, IPv4_ADDR_SIZE);
                        memcpy(new_route->next_hop, ruta_de_nuestra_tabla->next_hop, IPv4_ADDR_SIZE);
                        int metrica_nueva = ntohl(ripv2_msg->vectores_distancia[i].metric) +1 ;
                        new_route->metric = metrica_nueva;
                        new_route->timer_ripv2 = timeout;
                        }
                    } else { //Si no viene del padre
                        int metrica_nueva = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;

                        if(metrica_nueva < ruta_de_nuestra_tabla->metric)
                        { //Actualizamos la ruta
                            printf("La métrica nueva es menor, actualizamos la ruta\n");

                            memcpy(new_route->subnet_addr, ruta_de_nuestra_tabla->subnet_addr, IPv4_ADDR_SIZE);
                            memcpy(new_route->subnet_mask, ruta_de_nuestra_tabla->subnet_mask, IPv4_ADDR_SIZE);
                            memcpy(new_route->next_hop, ruta_de_nuestra_tabla->next_hop, IPv4_ADDR_SIZE);
                            new_route->metric = metrica_nueva;
                            new_route->timer_ripv2 = timeout;

                            rip_route_table_remove(rip_table, route_index);
                            rip_route_table_add(rip_table, new_route);
                            rip_route_table_print(rip_table);

                        }
                    }
                }
            }


//OPTIONAL
        //someone asks for our route
        } else if(ripv2_msg->type == RIPv2_REQUEST) {
            printf("\nRequest recieved\n");

            int numero_rutas = number_of_routes(rip_table); //Calcula el numero de rutas
            struct ripv2_msg paquete_rip;

            paquete_rip.type = RIPv2_RESPONSE;
            paquete_rip.version=2;
            paquete_rip.dominio_encaminamiento=0x0000;
            ripv2_route_t * rutas;
            for(int i = 0; i < numero_rutas; i++)
            {
                rutas = ripv2_route_table_get(rip_table, i);
                if(rutas != NULL)
                {
                    paquete_rip.vectores_distancia[i].id_familia = AF_INET; //2
                    paquete_rip.vectores_distancia[i].etiqueta = 0x0000;
                    memcpy(paquete_rip.vectores_distancia[i].subred, rutas->subnet_addr, IPv4_ADDR_SIZE);
                    memcpy(paquete_rip.vectores_distancia[i].subnet_mask, rutas->subnet_mask, IPv4_ADDR_SIZE);
                    memcpy(paquete_rip.vectores_distancia[i].next_hop, rutas->next_hop, IPv4_ADDR_SIZE);
                    paquete_rip.vectores_distancia[i].metric = htonl(rutas->metric);
                } //Si ruta es NULL
            }
            int total_longitud = (numero_rutas*20) + 4;
            udp_send(my_udp_layer, client_port, dest_ip, (unsigned char *) &paquete_rip, total_longitud);
        }
    }
    return 0;

    udp_close(my_udp_layer);
}

