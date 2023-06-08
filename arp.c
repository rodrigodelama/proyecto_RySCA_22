#include "arp.h"

#include <stdio.h>
#include <timerms.h>
#include "log.h"
mac_addr_t ARP_BCAST_ADDR = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
ipv4_addr_t RIPv2_ADDR_ARP = { 224, 0, 0, 9 };

//ARP request and reply handling
int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr, ipv4_addr_t my_ip_addr)
{
    #ifdef DEBUG
        char arp_debug[60];
        ipv4_addr_str(my_ip_addr, arp_debug);
        printf("IP sent to arp_resolver: %s", arp_debug);
    #endif
    
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
    //ipv4_str_addr("0.0.0.0", arp_header_t.src_IPv4_addr); //IP por defecto segun el pdf
    memcpy(arp_header_t.src_IPv4_addr, my_ip_addr, 4); //Tamaño dirs IP -> 4 bytes
    memcpy(arp_header_t.dest_IPv4_addr, ip_addr, 4); //Tamaño dirs IP -> 4 bytes
    memcpy(arp_header_t.dest_MAC_addr, ARP_BCAST_ADDR, MAC_ADDR_SIZE);

    char * name = eth_getname(iface); //Obtengo el nombre del manejador (dado por parámetro de la función)
    
    //LOGS
    char my_ip_str[60];
    ipv4_addr_str(arp_header_t.src_IPv4_addr, my_ip_str);
    char mac_str[60];
    mac_addr_str(arp_header_t.src_MAC_addr, mac_str);

    //Type de ARP = 0x0806
    //Envio ARP Request
    eth_send(iface, MAC_BCAST_ADDR, 0x0806, (unsigned char *) &arp_header_t, sizeof(struct arp_header));
    log_trace("ARP (REQUEST) packet sent from MAC -> %s  (Interface: %s) & IP -> %s as source IP.\n", mac_str,name,my_ip_str);
                    //0x0806 es el type code de ARP de capa superior.
                    //ARP and ETH MAC destination addresses dont match (ARP_BCAST_ADDR vs MAC_BCAST_ADDR)
    //Recibir el reply
    //variables para almacenar datos de eth_rcv
    mac_addr_t src_addr; 
    unsigned char buffer[ETH_MTU];

    long int timeout = 2000;
    timerms_t timer;
    timerms_reset(&timer, timeout);
    struct arp_header * arp_header_recv = NULL;

    int is_my_dest_ip;//checkea si es la ip de destino pasada como argumento de esta función.
    int is_my_src_ip;//checkea si es la ip de origen pasada como argumento de esta función.
    int is_request;
    int len;

    do
    {
        long int time_left = timerms_left(&timer);

        len = eth_recv(iface, src_addr, PROT_TYPE_ARP, buffer, ETH_MTU, time_left);

        if(len == -1)
        {
            //Si no hay datos
            fprintf(stderr, "ERROR: eth_recv() broke\n");
            return -1;
        }
        else if(len == 0)
        {
        //Parte Opcional 1:
            fprintf(stderr, "%s: ERROR: Temporizador de 2s agotado, no hay respuesta de la IP de destino\n", name);
            //No hemos recibido una respuesta en 2 segundos, reenviamos el ARP_request y ponemos un limite de tiempo de 3 segundos (3000ms)

            //Cambiamos el limite de tiempo.
            timeout = 3000;
            //Reenviamos ARP_request
            eth_send(iface, MAC_BCAST_ADDR, 0x0806, (unsigned char *) &arp_header_t, sizeof(struct arp_header));
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

        }

        /* Comprobar si es la trama que estamos buscando */
        arp_header_recv = (struct arp_header*) buffer;

        is_request = (ntohs(arp_header_recv -> opcode) == OPCODE_REPLY);
        is_my_dest_ip = (memcmp(arp_header_recv ->src_IPv4_addr, ip_addr, sizeof(ipv4_addr_t)) == 0);
        is_my_src_ip = (memcmp(arp_header_recv ->dest_IPv4_addr, my_ip_addr, sizeof(ipv4_addr_t)) == 0);
        
        //eth_recv ya checkea la MAC y el tipo de hardware para que sea Ethernet.
        //TODO: mirar si hay que checkear más campos.
    } while(!(is_my_dest_ip && is_request && is_my_src_ip)); //nos importa solo la ip de dest del sender y el opcode para que sea "request"
    memcpy(mac_addr, arp_header_recv -> src_MAC_addr, sizeof(mac_addr_t)); //whats the point? -> Exit parameter.
    //To print out the found out data:
    char* dest_ip_str = (char*) malloc(sizeof(ipv4_addr_t));
    ipv4_addr_str(ip_addr, dest_ip_str);
    char* dest_mac_str = (char*) malloc(sizeof(mac_addr_t));
    mac_addr_str(mac_addr, dest_mac_str);

    log_trace("At the end of arp_resolve();\n");
    log_trace("%s -> %s \n", dest_ip_str, dest_mac_str);

    //Necessary to clear memory after use (and to avoid warnings hehe)
    free(dest_ip_str);
    free(dest_mac_str);

    return 0;
}
