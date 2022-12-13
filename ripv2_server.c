#include "global_dependencies.h"

#include "ripv2.h"
#include "ripv2_route_table.h"

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
    uint16_t client_port;
    unsigned char buffer_rip[LEN_PAYLOAD_RIP]; //1472 de capacidad
    udp_layer_t * my_udp_layer = udp_open(server_port, "./ipv4_config_client.txt", "./ipv4_route_table_client.txt");
    log_trace("udp_layer configuration DONE\n");
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    ripv2_route_table_t * rip_table = rip_route_table_create(); //Creamos la tabla de rutas
    rip_route_table_read ("./ripv2_route_table_server.txt", rip_table); //Rellenamos la tabla
    rip_route_table_print(rip_table);

    while (1)
    {
        int bytes_rcvd = udp_rcv(my_udp_layer,dest_ip, &client_port, buffer_rip, sizeof(ripv2_msg_t), timeout);//udp ya nos devuelve el número de bytes útiles (no worries en teoría). 
        log_debug("Total number of bytes received -> %d \n", bytes_rcvd);
        int numero_de_vectores_distancia = (bytes_rcvd - RIPv2_MESSAGE_HEADER_SIZE) / RIPv2_DISTANCE_VECTOR_ENTRY_SIZE ;//deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
        log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);
        ripv2_msg_t* ripv2_response = (ripv2_msg_t*) buffer_rip;
        
    }
    















    //REQUEST:-----------------------------------------------------------------------------------------------------------------------------
    
    //! what do we do with config file? -> we will create a ripv2 file with just the route to our default gateway.
    udp_layer_t * my_udp_layer = udp_open(random_port_generator(), "./ipv4_config_client.txt", "./ipv4_route_table_client.txt");//para las rutas, seguiremos utilizando estáticas.
    log_trace("udp_layer configuration DONE\n");
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    log_trace("Building (REQUEST) message\n");    
    ripv2_msg_t request_message;//Si no hago el malloc, me dice que la variable no esta inicializada ??
    memset(&request_message, 0, sizeof(ripv2_msg_t));
    //Cabecera RIP:
    request_message.type = (uint8_t) 1;
    request_message.version = (uint8_t) 2;
    request_message.dominio_encaminamiento = htons((uint16_t) 0x0000);
    //Entrada 1, vector distancia:
    request_message.vectores_distancia[0].familia_dirs = htons((uint16_t) 0x0000);
    //log_debug("Familia_dirs");
    request_message.vectores_distancia[0].etiqueta_ruta = htons((uint16_t) 0x0000);
    memcpy(request_message.vectores_distancia[0].subred , IPv4_ZERO_ADDR_2, sizeof(ipv4_addr_t));
    memcpy(request_message.vectores_distancia[0].subnet_mask , IPv4_ZERO_ADDR_2, sizeof(ipv4_addr_t));
    memcpy(request_message.vectores_distancia[0].next_hop, IPv4_ZERO_ADDR_2, sizeof(ipv4_addr_t));
    request_message.vectores_distancia[0].metric = htonl((uint32_t) 16);
    

    //unsigned char* ripv2_request_payload = (unsigned char*) &request_message;
    int length_request = RIPv2_MESSAGE_HEADER_SIZE + (RIPv2_DISTANCE_VECTOR_ENTRY_SIZE * 1);
    log_debug("Length of request packet -> %d\n", length_request);
    int bytes_sent = udp_send(my_udp_layer, dest_ip, destport, (unsigned char*) &request_message, length_request);
    log_debug("Bytes of data sent by UDP send -> %d\n",bytes_sent);
    unsigned char fake_payload_rcv[1200];
    int timeout = 6000;
    int bytes_rcvd = udp_rcv(my_udp_layer,dest_ip, &destport, fake_payload_rcv, sizeof(ripv2_msg_t), timeout);//udp ya nos devuelve el número de bytes útiles (no worries en teoría). 
    log_debug("Total number of bytes received -> %d \n", bytes_rcvd);
    
    int numero_de_vectores_distancia = (bytes_rcvd - RIPv2_MESSAGE_HEADER_SIZE) / RIPv2_DISTANCE_VECTOR_ENTRY_SIZE ;//deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
    log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);

    ripv2_msg_t* ripv2_response = (ripv2_msg_t*) fake_payload_rcv;
    // no tenemos tabla en el cliente -> ripv2_msg_t* ripv2_response = (ripv2_msg_t*) fake_payload_rcv;
    // no recibimos tabla, solamente array de entradas, por tanto, usamos bucle "for" y ripv2_route_print() para cada posición "i" del array.  
    log_trace("Received table -> \n");
    //ripv2_route_table_print ( ripv2_response->vectores_distancia);
    for(int i = 0; i < numero_de_vectores_distancia; i++){
        log_trace("Vector distancia, posicion (%d) -> ", i);
        ripv2_vector_print(&(ripv2_response->vectores_distancia[i]));
    }
    if(bytes_rcvd == 0){
        log_trace("Reception timeout reached...\n\n");
    }else{
        log_trace("RESPONSE Packet received!!\n");
    }
    udp_close(my_udp_layer);
    return 0;

}
//no hace falta 
