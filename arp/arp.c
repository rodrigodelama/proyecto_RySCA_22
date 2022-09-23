#include "arp.h"
#include <stdio.h>
#include <timerms.h>
//extern mac_addr_t discovery_mac_addr; //FIXME: ASK about extern

struct arp_header
{
    //constants by the "defines"
    uint16_t hardware_type; //protocolo capa inferior (eth)
    uint16_t protocol_type; //protocolo capa superior (ipv4)
    uint8_t hw_size; //numero de bytes de las direcciones de la capa inferior (6 bytes en eth)
    uint8_t protocol_size; //numero de bytes de las direcciones de la capa superior (4 bytes en IPv4)

    uint16_t opcode; //request = 1, reply = 2
    mac_addr_t src_MAC_addr; //sender MAC address - format XX:XX:XX:XX:XX:XX
    ipv4_addr_t src_IPv4_addr; //sender IPv4 address - format xxx.xxx.xxx.xxx, where x is any int [0,255]
    mac_addr_t dest_MAC_addr; //target MAC address
    ipv4_addr_t dest_IPv4_addr; //target IPv4 address
};

//ARP request and reply handling
int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr)
{
    //Proveniente del "main" tenemos la interfaz y direccion IP convertidas a sus respectivos tipos
 
    struct arp_header arp_header_t; //Creo header de ARP
    
    //Empezamos a convertir y rellenar los campos de la cabecera ARP
    memset(&arp_header_t, 0, sizeof(struct arp_header)); //Relleno la zona de memoria que guarda nuestra cabecera ARP con 0s
    //Necesitamos convertir con htons() los de 16 bits, que son hardware_type y protocol_type
    arp_header_t.hardware_type = htons(HW_TYPE_ETH);
    arp_header_t.protocol_type = htons(PROT_TYPE_IPV4);
    //Podemos meter tal cual los valores de 1 byte (8 bits), que son hw_size y protocol_size
    arp_header_t.hw_size = (uint8_t) HW_SIZE_MAC_ADDR;
    arp_header_t.protocol_size = (uint8_t) PROT_SIZE_IP_ADDR;

    //Tambien convertiremos el opcode cuando lo mandemos (16 bits), esto habria que cambiarlo si fuesemos los solicitados
    arp_header_t.opcode = htons(OPCODE_REQUEST);

    //"Montamos" el resto paquete a mandar
    eth_getaddr(iface, arp_header_t.src_MAC_addr);
    ipv4_str_addr("0.0.0.0", arp_header_t.src_IPv4_addr); //IP por defecto segun el pdf
    memcpy(arp_header_t.dest_IPv4_addr, ip_addr, 4); //Tamaño dirs IP -> 4 bytes
    memcpy(arp_header_t.dest_MAC_addr, MAC_BCAST_ADDR, MAC_ADDR_SIZE);

    char * name = eth_getname(iface); //Obtengo el nombre del manejador (dado por parámetro de la función)
    
    //Type de ARP = 0x0806
    //Envio ARP Request
    eth_send(iface, arp_header_t.dest_MAC_addr, 0x0806, (unsigned char *) &arp_header_t, sizeof(struct arp_header));//0x0806 es el type code de ARP de capa superior.
    
    //Recibir el reply
    //variables para almacenar datos de eth_rcv
    mac_addr_t src_addr; 
    unsigned char buffer[ETH_MTU];
    
    /*
    int eth_buf_len = ETH_HEADER_SIZE + buf_len;
    unsigned char eth_buffer[eth_buf_len];
    */
    long int timeout = 2000;
    timerms_t timer;
    timerms_reset(&timer, timeout);
    struct arp_header * arp_header_recv = NULL;
    //int is_hardware_type;
    //int is_protocol_type;
    //int is_my_ip;//checckea si es para mi ip y no es broadcast.
    int is_my_dest_ip;//checkea si es la ip de destino pasada como argumento de esta función.
    int is_request;
    int len;
    do
    {
        long int time_left = timerms_left(&timer);

        len = eth_recv(iface, src_addr, PROT_TYPE_ARP, buffer, ETH_MTU, time_left);

        if (len == -1)
        {
            //Si no hay datos
            fprintf(stderr, "ERROR: eth_recv() broke\n");
            return -1;
        }
        else if (len == 0)
        {
        //Parte Opcional 1:
            fprintf(stderr, "%s: ERROR: Temporizador de 2s agotado, no hay respuesta de la IP de destino\n", name);
            //No hemos recibido una respuesta en 2 segundos, reenviamos el ARP_request y ponemos un limite de tiempo de 3 segundos (3000ms)

            //Cambiamos el limite de tiempo.
            timeout = 3000;
            //Reenviamos ARP_request
            eth_send(iface, arp_header_t.dest_MAC_addr, 0x0806, (unsigned char *) &arp_header_t, sizeof(struct arp_header));

            len = eth_recv(iface, src_addr, 0x0806, buffer, ETH_MTU, timeout); //Esperamos a recibir el ARP_reply.

            if (len == -1)
            {
                //Si no hay datos
                fprintf(stderr, "ERROR: eth_recv() broke\n");
                return -1;
            }
            else if (len == 0)
            {
                //Si pasan los 3 segundos.
                fprintf(stderr, "%s: ERROR: Temporizador de 3s agotado, cerrando conexion...\n", name);
                return -1;
            }

        }/*else if(frame_len < ETH_HEADER_SIZE){
            fprintf(stderr, "eth_recv(): Trama de tamaño invalido: %d bytes\n",frame_len);
            continue;
        }
        */
        /* Comprobar si es la trama que estamos buscando */
        arp_header_recv = (struct arp_header*) buffer;
        //is_my_mac = (memcmp(arp_header_recv->dest_MAC_addr,iface->mac_address, MAC_ADDR_SIZE) == 0);
        //is_my_ip = (memcmp(arp_header_recv ->dest_IPv4_addr, "0.0.0.0") == 0); no hace falta, hay que ser permisivo en este caso
        //is_hardware_type = (ntohs(arp_header_recv->hardware_type) == HW_TYPE_ETH);
        //is_protocol_type = (ntohs(arp_header_recv->hardware_type) == PROT_TYPE_IPV4);
        is_request = (ntohs(arp_header_recv -> opcode) == OPCODE_REPLY);
        is_my_dest_ip = (memcmp(arp_header_recv ->src_IPv4_addr, ip_addr,sizeof(ipv4_addr_t)) == 0);
        //eth_recv ya checkea la MAC y el tipo de hardware para que sea Ethernet.
        //TODO: mirar si hay que checkear más campos.
    } while(!(is_my_dest_ip && is_request));//nos importa solo la ip de dest del sender y el opcode para que sea "request"
    

    //struct arp_header * arp_header_recv = (struct arp_header *) buffer;


    //FIXME: potentially unnecessary - check with the pdf: 20202021_RYSCA_enunciado_cliente_ARP.pdf
    memcpy(mac_addr, arp_header_recv -> src_MAC_addr, sizeof(mac_addr_t)); //whats the point?

    //Copied to our global var for future use
    memcpy(discovery_mac_addr, mac_addr, sizeof(mac_addr_t));
    
    //To print out the found out data:
    char* dest_ip_str = (char*) malloc(sizeof(ipv4_addr_t));
    ipv4_addr_str(ip_addr, dest_ip_str);
    char* dest_mac_str = (char*) malloc(sizeof(mac_addr_t));
    mac_addr_str(mac_addr, dest_mac_str);

    printf("%s -> %s \n", dest_ip_str, dest_mac_str);

    //Necessary to clear memory after use (and to avoid warnings hehe)
    free(dest_ip_str);
    free(dest_mac_str);

    return 0;
}

int main(int argc, char* argv[])
{

    struct arp_header arp_header_t;
    //Comprobaciones del numero correcto de argumentos y si son correctos.
    memset(&arp_header_t, 0, sizeof(struct arp_header));//Relleno la zona de memoria que guarda nuestra cabecera ARP con 0. 

    if(argc != 3 )
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso:  <iface> <target_ip>\n");
        printf("      <iface>: Nombre de la interfaz Ethernet\n");
        printf("      <target_ip>: Direccion ip para solicitar su MAC \n");
        exit(-1);
    }
    
    char* iface_name = argv[1];
    eth_iface_t* iface_controller = eth_open(iface_name);
    if(eth_open(iface_name) == NULL) //Checking if the interface is a valid one
    {
        fprintf(stderr, "%s\n", "Error con la interfaz\n");
        exit(-1);
    }

    ipv4_addr_t target_ip;
    if (ipv4_str_addr(argv[2], target_ip) == -1)
    {
        fprintf(stderr, "\nInvalid target IP Address");
        exit(-1);
    }
    //A partir de aqui, los parametros pasados por linea de comandos estaran en el formato correcto
    arp_resolve(iface_controller, target_ip, discovery_mac_addr); //mac_addr should be the thing to recover!!

    eth_close(iface_controller); //Cerramos la interfaz al terminar, en el futuro nos quedaremos solo con arp_resolve(). 
}
