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
      printf("el temporizador  %d :%ld\n", i, time_left );
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

int set_next_hop(ipv4_addr_t next_hop)
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

    /*ipv4_addr_t dest_ip;//In this case, this has to be our gateway addr (router address). 

    if(ipv4_str_addr(argv[1], dest_ip) == -1)
    {
        fprintf(stderr, "%s\n", "IP pasada como parámetro no válida\n");
        exit(-1);
    }
    if(atoi(argv[2]) == 0 || atoi(argv[2]) < 0){
        fprintf(stderr, "%s\n", "Puerto de destino pasado como parámetro no es válido\n");
        exit(-1);
    }
    */
    ipv4_addr_t dest_ip;
    uint16_t server_port = RIPv2_PORT;
    int i = 0;
    uint16_t client_port;
    int index_min;
    unsigned char buffer_rip[LEN_PAYLOAD_RIP]; //?????????? cuanto? 1400 maybe
    
    udp_layer_t * my_udp_layer = udp_open(server_port, "./ipv4_config_client.txt", "./ipv4_route_table_client.txt");
        log_trace("udp_layer configuration DONE\n");
    
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    ripv2_route_table_t * rip_table = ripv2_route_table_crate(); //Creamos la tabla de rutas
    ripv2_route_table_read ("./ripv2_route_table_server.txt", rip_table); //Rellenamos la tabla
    ripv2_route_table_print(rip_table);

    int is_our_route;
    
    while (1)
    {
        long int timeout = least_time(rip_table);

        int bytes_rcvd = udp_rcv(my_udp_layer,dest_ip, &client_port, buffer_rip, sizeof(ripv2_msg_t), timeout); //udp ya nos devuelve el número de bytes útiles (no worries en teoría). 
            log_debug("Total number of bytes received -> %d \n", bytes_rcvd);
        
        int numero_de_vectores_distancia = (bytes_rcvd - RIPv2_MESSAGE_HEADER_SIZE) / RIPv2_DISTANCE_VECTOR_ENTRY_SIZE ; //deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
            log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);
        
        ripv2_msg_t* ripv2_response = (ripv2_msg_t*) buffer_rip;
        //expire_time + index_min es sobre todo para encontrar la ruta que ha expirado, y poder seleccionarla con la funcion "ripv2_route_table_get()" 
        // y (llegado el caso) eliminarla con la funcion ripv2_route_table_remove(). -> es sobre todo para tenerla localizada.
        long int expire_time = least_time(rip_table);//Menor temporizador de entre las rutas que tenemos
        index_min = index_least_time(rip_table); //indice (posición en la tabla) de la ruta con menor temporizador
        
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
            for(i = 0; i < numero_de_vectores_distancia; i++)
            {
                int index_min;
                int index_ruta = 0;
                struct ripv2_msg* ripv2_msg_rcvd = NULL;
                index_ruta = ripv2_route_table_find(rip_table, ripv2_msg_rcvd->vectores_distancia[i].subred, ripv2_msg_rcvd->vectores_distancia[i].subnet_mask);
                if(index_min == index_ruta)
                {
                    is_our_route = 1;
                }
            }

            if(is_our_route != 1)
            {
                printf("Eliminamos la ruta %s\n",subnet_str);
                is_our_route = 0;
                rip_route_table_remove(rip_table,index_min);
                rip_route_table_print(rip_table);
            }
        }

    }
    return 0;

    udp_close(my_udp_layer);
}
