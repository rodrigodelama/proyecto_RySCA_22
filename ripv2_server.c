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
    #ifdef DEBUG
        ripv2_route_table_print(rip_table);
    #endif

    int index_min;
    int is_our_route;
    int route_index;
    ipv4_addr_t next_hop;
    int calc_gateway;
    char rip_iface[10] = "eth1";

    //TTL is part of IPv4 and to change it we would need a more profound overhaul than simply making the request

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
    

    //unsigned char* ripv2_request_payload = (unsigned char*) &request_message;
    int length_request = RIPv2_MESSAGE_HEADER_SIZE + (RIPv2_DISTANCE_VECTOR_ENTRY_SIZE * 1);
    log_debug("Length of request packet -> %d\n", length_request);
    int bytes_sent = udp_send(my_udp_layer, RIPv2_ADDR_2, RIPv2_PORT, (unsigned char*) &request_message, length_request);
    log_debug("Bytes of data sent by UDP send -> %d\n",bytes_sent);

    while (1)
    {
        //TODO:
        //check if empty table and loop awaiting a new entry
        //if table entries time out default to the listening loop

        ripv2_route_table_print(rip_table);
        printf("\n");
        
        long int timeout = least_time(rip_table);
        //En la primera iteración, tenemos tabla con cosas.
        int bytes_rcvd = udp_rcv(my_udp_layer, source_ip, &client_port, buffer_rip, sizeof(ripv2_msg_t), timeout); //udp ya nos devuelve el número de bytes útiles (no worries en teoría). 
            log_debug("Total number of bytes received -> %d \n", bytes_rcvd);
        //SIEMPRE hay que mirar el origen del mensaje, si es mi padre, solamente actualizo la metrica y reseteo el timer. Si no viene de mi padre (mi garteway)
        //hay que mirar tambien si la métrica es mejor y si eso actualizar.

        int numero_de_vectores_distancia = (bytes_rcvd - RIPv2_MESSAGE_HEADER_SIZE) / RIPv2_DISTANCE_VECTOR_ENTRY_SIZE ; //deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
            log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);
        
        ripv2_msg_t* ripv2_msg = (ripv2_msg_t*) buffer_rip;
        // expiration_time + index_min es sobre todo para encontrar la ruta que ha expirado, y poder seleccionarla con la funcion "ripv2_route_table_get()" 
        // y (llegado el caso) eliminarla con la funcion ripv2_route_table_remove(). -> es sobre todo para tenerla localizada.
        long int expiration_time = least_time(rip_table); // route with smallest timer found
        index_min = index_least_time(rip_table); // route

        ripv2_route_t* current_route = ripv2_route_table_get(rip_table, index_min); //ruta con menor temporizador
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

        } else if (bytes_rcvd > 0 && expiration_time == 0) { //deberia estar bien -> comprobar
            for(int i = 0; i < numero_de_vectores_distancia; i++)
            {
                // int index_min;
                route_index = 0;
                route_index = ripv2_route_table_find(rip_table, ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask);
                //devuelve el índice de la ruta para llegar a la subred especificada.
                if(index_min == route_index)
                {
                    is_our_route = 1;
                }
            }
            // this if might not be needed
            if(is_our_route != 1)
            {
                log_trace("Deleted the route: %s", subnet_str);
                is_our_route = 0;
                ripv2_route_table_remove(rip_table, index_min);
                ripv2_route_table_print(rip_table);
            }
        }

        // someone sends us their routes
        if(ripv2_msg->type == RIPv2_RESPONSE) // no hace falta hacer noths porque es un entero de 8bits (1 byte).
        {
            char str_route[IPv4_STR_MAX_LENGTH];
                log_trace("Received %d distance vectors", numero_de_vectores_distancia);

            for(int i = 0; i < numero_de_vectores_distancia; i++)
            {
                ipv4_addr_str(ripv2_msg->vectores_distancia[i].subred, str_route);

                log_trace("Route %d is %s\n ", i, str_route);

                route_index = ripv2_route_table_find(rip_table, ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask);

                if(route_index == -1) // case in which we dont have the route inside our ripv2_table.
                {
                        log_trace("We dont have the route, we will create a new entry for it");
                    int metric_rcvd = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;

                    if(metric_rcvd < 16) //
                    {
                        memcpy(next_hop, ripv2_msg->vectores_distancia[i].next_hop, IPv4_ADDR_SIZE);
                        calc_gateway = set_gateway(ripv2_msg->vectores_distancia[i].next_hop); //del campo next_hop del vector distancia, razonamos el gateway
                        //Si devuelve 0, la gateway ha de ser la IP de donde lo hemos recibido, sino, gateway = next_hop.
                        if(calc_gateway == 0)
                        {
                            // next_hop = source_ip; // addr recieved as next hop is different from 0.0.0.0, we will use the source ip
                            memcpy(next_hop, source_ip, IPv4_ADDR_SIZE);
                        }
                        
                        ripv2_route_t * new_route;
                        new_route = ripv2_route_create(ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask, rip_iface, next_hop, metric_rcvd);
                            log_trace("Adding new route to our table.\n");
                        ripv2_route_table_add(rip_table, new_route);
                        // ripv2_route_table_print(rip_table);
                    } else {
                        log_trace("Distance for us is infinity, route discarded"); //only for routes of 15 or less hops
                    }

                // we already have the route
                } else if (route_index > -1) { //SI esta en la tabla
                    log_trace("Received a route we already have, we will check if we must update it");
                    
                    //mirar de quien viene
                        //caso papa
                            //mirar metrica recibida si 15 o superior, BORRAR (15+1 = inf, 16+1 = inf)
                            //SINO actualizamos la ruta con la nueva metrica (aunq sea peor), y reiniciamos el timer
                        //caso random
                            //comparar metrica recibida con la actual
                            //si es inferior, actualizar la ruta con la nueva metrica mejor
                    
                    //ripv2_route_table_remove(rip_table, route_index);
                    //ripv2_route_t * route_to_update = ripv2_route_create(ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask, rip_iface, ipv4_addr_t gw, new_metric);
                    //rip_route_table_add(rip_table, route_to_update);
                    
                    ripv2_route_t * route_to_update = (ripv2_route_t *) malloc(sizeof(ripv2_route_t));
                    //memset(&route_to_update, 0, sizeof(ripv2_route_t));
                    //ripv2_route created from dv recieved in ripv2_msg
                    memcpy(route_to_update->subnet_addr, ripv2_msg->vectores_distancia[i].subred, IPv4_ADDR_SIZE);
                    memcpy(route_to_update->subnet_mask, ripv2_msg->vectores_distancia[i].subnet_mask, IPv4_ADDR_SIZE);
                    uint32_t new_metric = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;
                    route_to_update->metric = new_metric;
                    memcpy(route_to_update->gateway_addr, ripv2_msg->vectores_distancia[i].next_hop, IPv4_ADDR_SIZE);

                    if(set_gateway(ripv2_msg->vectores_distancia[i].next_hop) == 0)
                    {
                        memcpy(route_to_update->gateway_addr, source_ip, IPv4_ADDR_SIZE);
                    }
                    
                    ripv2_route_t * registered_route = (ripv2_route_t *) malloc(sizeof(ripv2_route_t));
                    registered_route = ripv2_route_table_get(rip_table, route_index);

                    // si es del papa -> actualizamos a metrica sin impotar si es menor o mayor que la anterior.
                    //set_gateway(ipv4_addr_t next_hop);
                    if(memcmp(source_ip, registered_route->gateway_addr, IPv4_ADDR_SIZE) == 0)
                    {
                        if(new_metric >= 16) //si es de gw y tiene 16 o mas de metrica
                        {
                            registered_route->metric = 16;
                            ripv2_route_table_print(rip_table);
                            ripv2_route_table_remove(rip_table, route_index);
                            printf("\n");
                        } else { // si su metrica es inferior a 16 (aunque sea peor)
                            //update metric, whatever cost
                            registered_route->metric = new_metric; //simple type so equals
                            timerms_reset(&registered_route->timer_ripv2, RECEPTION_TIMER); //refresh timer
                        }
                    } else { // si no es del papa
                        if(new_metric < registered_route->metric) // actualizamos la ruta
                        {
                            log_trace("Better route recieved: We will update it with the latest info (not from our father router)\n");
                            
                            memcpy(registered_route->subnet_addr, route_to_update->subnet_addr, IPv4_ADDR_SIZE); //update address
                            memcpy(registered_route->subnet_mask, route_to_update->subnet_mask, IPv4_ADDR_SIZE); //update subnet
                            memcpy(registered_route->gateway_addr, route_to_update->gateway_addr, IPv4_ADDR_SIZE); //update next hop as source_ip
                            route_to_update->metric = new_metric; //update with better (lower) metric
                            timerms_reset(&registered_route->timer_ripv2, RECEPTION_TIMER); //refresh timer

                            // POSSIBLY EASIER : but its not updating a route
                            //rip_route_table_remove(rip_table, route_index);

                            //rip_route_table_add(rip_table, route_to_update);

                            // ripv2_route_table_print(rip_table);
                        }
                    }
                }
            }
        //someone asks for our routes
        } else if(ripv2_msg->type == RIPv2_REQUEST) {
            log_trace("Request recieved");

            int num_of_routes = number_of_routes(rip_table); //Calcula el numero de rutas
            ripv2_msg_t ripv2_msg;

            ripv2_msg.type = RIPv2_RESPONSE;
            ripv2_msg.version = 2;
            ripv2_msg.dominio_encaminamiento = 0x0000;
            ripv2_route_t * routes_to_send;
            for(int i = 0; i < num_of_routes; i++)
            {
                routes_to_send = ripv2_route_table_get(rip_table, i);
                if(routes_to_send != NULL)
                {
                    ripv2_msg.vectores_distancia[i].familia_dirs = AF_INET; //2
                    ripv2_msg.vectores_distancia[i].etiqueta_ruta = 0x0000;
                    memcpy(ripv2_msg.vectores_distancia[i].subred, routes_to_send->subnet_addr, IPv4_ADDR_SIZE);
                    memcpy(ripv2_msg.vectores_distancia[i].subnet_mask, routes_to_send->subnet_mask, IPv4_ADDR_SIZE);
                    memcpy(ripv2_msg.vectores_distancia[i].next_hop, routes_to_send->gateway_addr, IPv4_ADDR_SIZE);
                    ripv2_msg.vectores_distancia[i].metric = htonl(routes_to_send->metric);
                } //Si ruta es NULL
            }
            int total_len = (num_of_routes*20) + 4;

            // unsigned char * dest[IPv4_STR_MAX_LENGTH];
            // ipv4_addr_str(source_ip, dest);

            udp_send(my_udp_layer, source_ip, client_port, (unsigned char *) &ripv2_msg, total_len);

        }
        ripv2_route_table_print(rip_table);
    }
    udp_close(my_udp_layer);

    return 0;
}
