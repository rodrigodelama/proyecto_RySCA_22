# include "arp_client.h"

#define MAC_STR_SIZE  17

struct arp_header
{
    uint16_t hardware_type; //protocolo capa inferior (eth)
    uint16_t protocol_type; //protocolo capa superior (ipv4)
    uint8_t hw_size; //numero de bytes de las direcciones de la capa inferior (6 bytes en eth)
    uint8_t protocol_size; //numero de bytes de las direcciones de la capa superior (4 bytes en IPv4)
    uint16_t opcode; //request = 1, reply = 2
    mac_addr_t src_MAC_addr; //sender MAC address - format XX:XX:XX:XX:XX:XX
    ipv4_addr_t src_IPv4_addr; //sender IPv4 address - format xxx.xxx.xxx.xxx, where x is any int [0,255]
    mac_addr_t dest_MAC_addr; //target MAC address
    ipv4_addr_t dest_IPv4_addr; //target IPv4 address
} arp_header;


int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr)
{
    //ARP - Address Resolution Protocol
    //respond MAC when asked who has that IP Address

    //Creamos variables auxiliares
    char y[MAC_STR_SIZE];
    //Rellenamos la estructura con los datos correspondientes
    //Comprobamos que la mac introducida se corresponde con una mac 
    //y = mac_str_addr(dest_MAC_addr, arp_header.dest_addr_mac); 

    // Abrimos la interfaz Ethernet

    // Enviamos tramas Ethernet 
    // Inicializamos el temporizador para mantener timeout si se reciben tramas con tipo incorrecto.
    do
    {
    //Recibimos tramas Ethernet
    //Comprobamos que tenga un tamaño correcto 
    // Comprobamos si es la trama que estamos buscando 
    } while (); 
    //Hasta que sea la trama buscada
}

int main(int argc, char* argv[])
{
    //Comprobaciones de nº correcto de argumentos y si son correctos.
    
}