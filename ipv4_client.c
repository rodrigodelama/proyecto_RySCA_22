#include "ipv4.h"

#include "global_dependencies.h"
#include "ipv4_dependencies.h"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso: <target_ip>\n");
        printf("     <target_ip>: Direccion ip para solicitar su MAC \n");
        exit(-1);
    }
    ipv4_addr_t dest_ip;

    if (ipv4_str_addr(argv[1], dest_ip) == -1)
    {
        fprintf(stderr, "%s\n", "IP pasada como parámetro no válida\n");
        exit(-1);
    }

    /*
    # ipv4_config_client.txt
    #
    Interface eth0
    IPv4Address 192.168.1.100
    SubnetMask 255.255.255.0
    */
    ipv4_layer_t* my_ip_iface = ipv4_open("ipv4_config_client.txt", "ipv4_route_table_client.txt");
    if(my_ip_iface == NULL){
        fprintf(stderr, "%s\n", "Error abriendo interfaz IP Layer.\n");
        exit(-1);
    }
    
    
    unsigned char fake_payload[1460] = 0;

    struct ipv4_header ipv4_header_t;
    memset(&ipv4_header_t, 0, sizeof(struct ipv4_header)); //Relleno la zona de memoria que guarda nuestra cabecera IP con 0s
    
    /* Rellenamos sus campos */
    ipv4_header_t.version_and_length = (uint8_t) VERSION_AND_LENGTH; //"dos campos de 4bytes" rellenado a mano en Hex
    ipv4_header_t.service_type = 0;
    ipv4_header_t.total_length = (uint16_t) IPV4_HDR_LEN + sizeof(fake_payload); //total length is hdr + data
    ipv4_header_t.identification = (uint16_t) ID;
    ipv4_header_t.frag_flags = (uint16_t) 0;
    ipv4_header_t.ttl = (uint8_t) TTL_DEF;
    ipv4_header_t.protocol = (uint8_t) 17; //UDP
    ipv4_header_t.checksum = (uint8_t) 0; //initally at 0
    memcpy(ipv4_header_t.src_ip, my_ip_iface->addr, sizeof(ipv4_addr_t)); 
    memcpy(ipv4_header_t.dest_ip, dest_ip, sizeof(ipv4_addr_t));
    memcpy(ipv4_header_t.payload, fake_payload, sizeof(fake_payload));
    //Calculo de checksum:
    ipv4_header_t.checksum = ipv4_checksum( (unsigned char *) &ipv4_header_t, IPV4_HDR_LEN); // IPV4_HDR_LEN defined in ipv4.h 
                                                            // 1500 ETH - 20 cab IP = 1480
    ipv4_send(my_ip_iface, ipv4_header_t.dest_ip, ipv4_header_t.protocol, ipv4_header_t.payload, ipv4_header_t.total_length);
    
    
    return 0;
}

