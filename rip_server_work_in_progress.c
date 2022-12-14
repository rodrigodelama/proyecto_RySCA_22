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

long int least_time(ripv2_route_table_t * rip_table)
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
            log_trace("the timer %d has left: %ld\n", i, time_left );
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

    ipv4_addr_t source_ip; // from where the recieved packet came
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
    ipv4_addr_t* next_hop;
    int calc_gateway;
    char rip_iface[10] = "eth1";
    
    while (1)
    {
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

        } else if(bytes_rcvd == 0) { // we will eliminate bca timer is up
            log_trace("Timer's up, route %s has been eliminated", subnet_str);
            ripv2_route_table_remove(rip_table, index_min);
            // #ifdef DEBUG
                ripv2_route_table_print(rip_table);
            // #endif

        } else if(bytes_rcvd > 0 && expiration_time == 0) { //deberia estar bien -> comprobar
            for(int i = 0; i < numero_de_vectores_distancia; i++)
            {
                // int index_min;
                route_index = 0;
                route_index = ripv2_route_t_find(rip_table, ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask);
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
            printf("Received %d distance vectors", numero_de_vectores_distancia);
            for(int i = 0; i < numero_de_vectores_distancia; i++)
            {
                ipv4_addr_str(ripv2_msg->vectores_distancia[i].subred, str_route);

                printf("Route %d is %s\n ", i, str_route);

                route_index = ripv2_route_table_find(rip_table, ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask);

                if(route_index == -1) // case in which we dont have the route inside our ripv2_table.
                {
                    log_trace("We dont have the route, we will create a new entry for it");
                    int metric_rcvd = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;

                    if(metric_rcvd < 16) //
                    {

                        next_hop = ripv2_msg->vectores_distancia[i].next_hop;
                        calc_gateway = set_gateway(ripv2_msg->vectores_distancia[i].next_hop); //del campo next_hop del vector distancia, razonamos el gateway
                        //Si devuelve 0,la gateway ha de ser la IP de donde lo hemos recibido, sino, gateway = next_hop.
                        if(calc_gateway == 0)
                        {
                            next_hop = source_ip; // addr recieved as next hop is different from 0.0.0.0, we will use the source ip
                        }
                        
                        ripv2_route_t * new_route;
                        new_route = ripv2_route_create(ripv2_msg->vectores_distancia[i].subred, ripv2_msg->vectores_distancia[i].subnet_mask, rip_iface, next_hop, metric_rcvd);
                        log_trace("Adding new route to our table.\n");
                        rip_route_table_add(rip_table, new_route);
                        rip_route_table_print(rip_table);
                    } else {
                        log_trace("Distance for us is infinity, route discarded"); //only for routes of 15 or less hops
                    }

                // we already have the route
                } else if(route_index > -1) { //SI esta en la tabla
                    log_trace("Recieved a route we already have, we will check if we must update it");

                    //khe asemoh papi

                    //mirar de quien viene
                        //caso papa
                            //mirar metrica recibida si 15 o superior, BORRAR (15+1 = inf, 16+1 = inf)
                            //SINO actualizamos la ruta con la nueva metrica (aunq sea peor), y reiniciamos el timer
                        //caso random
                            //comparar metrica recibida con la actual
                            //si es inferior, actualizar la ruta con la nueva metrica mejor

                    ripv2_route_t * route_to_update;
                    ripv2_route_t * registered_route = ripv2_route_table_get(rip_table, route_index);

                    // si es del papa -> actualizamos a metrica sin impotar si es menor o mayor que la anterior.
                    if(memcmp(ripv2_msg->vectores_distancia[i].next_hop, registered_route->next_hop, IPv4_ADDR_SIZE) == 0)
                    {
                        if(ntohl(ripv2_msg->vectores_distancia[i].metric) + 1 >= 16) //si es de gw y tiene 16 o mas de metrica
                        {
                            rip_route_table_remove(rip_table,route_index);
                        } else { // si su metrica es inferior a 16 (aunque sea peor)
                            int new_metric = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;
                            route_to_update->metric = new_metric; //update metric, whatever cost
                            route_to_update->timer_ripv2 = timeout; //refresh timer
                        }
                    } else { // si no es del papa
                        int new_metric = ntohl(ripv2_msg->vectores_distancia[i].metric) + 1;

                        if(new_metric < registered_route->metric) // Actualizamos la ruta
                        {
                            log_trace("Better route detected: We will update it with the latest info (not from our father router)\n");

                            memcpy(route_to_update->subnet_addr, registered_route->subnet_addr, IPv4_ADDR_SIZE); //update address
                            memcpy(route_to_update->subnet_mask, registered_route->subnet_mask, IPv4_ADDR_SIZE); //update subnet
                            memcpy(route_to_update->next_hop, registered_route->next_hop, IPv4_ADDR_SIZE); //update next hop as source_ip
                            route_to_update->metric = new_metric; //update with better (lower) metric
                            route_to_update->timer_ripv2 = timeout; //refresh timer

                            // POSSIBLY EASIER : but its not updating a route
                            //rip_route_table_remove(rip_table, route_index);
                            //rip_route_table_add(rip_table, route_to_update);

                            rip_route_table_print(rip_table);
                        }
                    }
                }
            }


//OPTIONAL
        //someone asks for our route
        } else if(ripv2_msg->type == RIPv2_REQUEST) {
            log_trace("Request recieved");

            int num_of_routes = number_of_routes(rip_table); //Calcula el numero de rutas
            ripv2_msg_t ripv2_msg;

            ripv2_msg.type = RIPv2_RESPONSE;
            ripv2_msg.version=2;
            ripv2_msg.dominio_encaminamiento=0x0000;
            ripv2_route_t * rutas;
            for(int i = 0; i < num_of_routes; i++)
            {
                rutas = ripv2_route_table_get(rip_table, i);
                if(rutas != NULL)
                {
                    ripv2_msg.vectores_distancia[i].id_familia = AF_INET; //2
                    ripv2_msg.vectores_distancia[i].etiqueta = 0x0000;
                    memcpy(ripv2_msg.vectores_distancia[i].subred, rutas->subnet_addr, IPv4_ADDR_SIZE);
                    memcpy(ripv2_msg.vectores_distancia[i].subnet_mask, rutas->subnet_mask, IPv4_ADDR_SIZE);
                    memcpy(ripv2_msg.vectores_distancia[i].next_hop, rutas->next_hop, IPv4_ADDR_SIZE);
                    ripv2_msg.vectores_distancia[i].metric = htonl(rutas->metric);
                } //Si ruta es NULL
            }
            int total_longitud = (num_of_routes*20) + 4;
            udp_send(my_udp_layer, client_port, source_ip, (unsigned char *) &ripv2_msg, total_longitud);
        }
    }
    return 0;

    udp_close(my_udp_layer);
}
