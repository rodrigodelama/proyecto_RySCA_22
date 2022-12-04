#include "global_dependencies.h"
#include "ripv2.h"

ipv4_addr_t IPv4_ZERO_ADDR = { 0, 0, 0, 0 };
//only recieve operations to port 

/*

Enviar una petición de toda la tabla de entradas
El programa recibe como parámetros por línea de comandos:
Ficheros de configuración IPv4
Dirección IPv4 del servidor/router
Imprime por pantalla los RTEs (vectores distancia) recibidos
Probad primero contra los router

*/
/*
REQUEST: hacemos un open con un puerto cualquiera, construimos el request y lo mandamos, cerramos interfaz y abrimos otra que escuche en el
puerto 520:
    - escuche en el puerto 520. (ESTO PARA EL SERVER)
    -Recivimos la tabla 

*/

//Client will send UDP messages to a determined ip add, to a certain port there

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

    ipv4_addr_t dest_ip;//In this case, this has to be our gateway addr (router address). 

    if(ipv4_str_addr(argv[1], dest_ip) == -1)
    {
        fprintf(stderr, "%s\n", "IP pasada como parámetro no válida\n");
        exit(-1);
    }
    if(atoi(argv[2]) == 0 || atoi(argv[2]) < 0){
        fprintf(stderr, "%s\n", "Puerto de destino pasado como parámetro no es válido\n");
        exit(-1);
    }
    //REQUEST:-----------------------------------------------------------------------------------------------------------------------------
    uint16_t destport = 520;
    //! what do we do with config file? -> we will create a ripv2 file with just the route to our default gateway.
    udp_layer_t * my_udp_layer = udp_open(random_port_generator(), "./ipv4_config_client.txt", "./ripv2_route_table_client.txt");
    if(my_udp_layer == NULL)
    {
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }    
    ripv2_msg_t* request_message;
    request_message->type = (uint8_t) 1;
    request_message->version = (uint8_t) 2;
    request_message->dominio_encaminamiento = htons((uint16_t) 0x0000);
    
    request_message->vectores_distancia[0].familia_dirs = htons((uint16_t) 0x0000);
    request_message->vectores_distancia[0].etiqueta_ruta = htons((uint16_t) 0x0000);
    memcpy(request_message->vectores_distancia[0].subred , htonl(IPv4_ZERO_ADDR), sizeof(ipv4_addr_t));
    memcpy(request_message->vectores_distancia[0].subnet_mask , htonl(IPv4_ZERO_ADDR), sizeof(ipv4_addr_t));
    memcpy(request_message->vectores_distancia[0].next_hop, htonl(IPv4_ZERO_ADDR), sizeof(ipv4_addr_t));
    request_message->vectores_distancia[0].metric = htonl((uint32_t) 16);
    

    unsigned char* ripv2_request_payload = (unsigned char*) request_message;
    int bytes_sent = udp_send(my_udp_layer, dest_ip, destport, ripv2_request_payload, (RIPv2_MESSAGE_HEADER_SIZE + (RIPv2_DISTANCE_VECTOR_ENTRY_SIZE * 1)));//Solamente queremos que mande ahora mismo la cabecera udp.
    log_debug("Bytes of data sent by UDP send -> %d\n",bytes_sent);
    unsigned char fake_payload_rcv[1200];
    int timeout = 6000;
    int bytes_rcvd = udp_rcv(my_udp_layer,dest_ip, &destport, fake_payload_rcv, sizeof(ripv2_msg_t), timeout);//udp ya nos devuelve el número de bytes útiles (no worries en teoría). 
    log_debug("Total number of bytes received -> %d \n", bytes_rcvd);
    
    int numero_de_vectores_distancia = (bytes_rcvd - 8) / 20 ;//deberíamos tener como resultado un entero, así sabremos hasta qué posición de la tabla tenemos que iterar en el "for". 
    log_debug("Number of table entrys received -> %d \n", numero_de_vectores_distancia);



    ripv2_msg_t* ripv2_response = (ripv2_msg_t*) fake_payload_rcv;
    // no tenemos tabla en el cliente -> ripv2_msg_t* ripv2_response = (ripv2_msg_t*) fake_payload_rcv;
    // no recibimos tabla, solamente array de entradas, por tanto, usamos bucle "for" y ripv2_route_print() para cada posición "i" del array.  
    log_trace("Received table -> \n");
    //ripv2_route_table_print ( ripv2_response->vectores_distancia);
    for(int i = 0; i < numero_de_vectores_distancia; i++){
        log_trace("Vector distancia, posicion (%d) -> ", i);
        ripv2_route_print( ripv2_response->vectores_distancia[i]);
    }
    if(bytes_rcvd == 0){
        log_trace("Reception timeout reached...\n\n");
    }else{
        log_trace("ECHO Packet received!!\n");
    }
    udp_close(my_udp_layer);
    return 0;

}
//no hace falta 
