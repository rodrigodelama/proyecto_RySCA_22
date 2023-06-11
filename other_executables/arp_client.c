#include "arp.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    mac_addr_t discovery_mac_addr; //MAC Address to be "discovered" by our ARP request
    
    //struct arp_header* arp_header_t = (struct arp_header*) malloc(sizeof(struct arp_header));
    struct arp_header arp_header_t; //Creo header de ARP
    //Comprobaciones del numero correcto de argumentos y si son correctos.
    memset(&arp_header_t, 0, sizeof(struct arp_header));//Relleno la zona de memoria que guarda nuestra cabecera ARP con 0. 

    if(argc != 3)
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
        fprintf(stderr, "\nDireccion IP no valida");
        exit(-1);
    }
    ipv4_addr_t my_ip;
    ipv4_str_addr("0.0.0.0", my_ip); //IP por defecto segun el pdf
    //A partir de aqui, los parametros pasados por linea de comandos estaran en el formato correcto
    arp_resolve(iface_controller, target_ip, discovery_mac_addr, my_ip); //mac_addr should be the thing to recover!!
    char discovery_mac_addr_str[MAC_STR_LENGTH];
    mac_addr_str ( discovery_mac_addr, discovery_mac_addr_str);
    
    // UNNECESSARY same as above
    //printf("¿Se rellena la mac adecuadamente? -> %s\n", discovery_mac_addr_str);
    
    eth_close(iface_controller); //Cerramos la interfaz al terminar, en el futuro nos quedaremos solo con arp_resolve(). 
}
