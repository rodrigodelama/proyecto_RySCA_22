# include "arp_client.h"

#define MAC_STR_SIZE  17

struct arp_header
{
    //TODO: constants
    uint16_t hardware_type; //protocolo capa inferior (eth)
    uint16_t protocol_type; //protocolo capa superior (ipv4)
    uint8_t hw_size; //numero de bytes de las direcciones de la capa inferior (6 bytes en eth)
    uint8_t protocol_size; //numero de bytes de las direcciones de la capa superior (4 bytes en IPv4)
    //^^

    uint16_t opcode; //request = 1, reply = 2
    mac_addr_t src_MAC_addr; //sender MAC address - format XX:XX:XX:XX:XX:XX
    ipv4_addr_t src_IPv4_addr; //sender IPv4 address - format xxx.xxx.xxx.xxx, where x is any int [0,255]
    mac_addr_t dest_MAC_addr; //target MAC address
    ipv4_addr_t dest_IPv4_addr; //target IPv4 address
} arp_header_t;


int arp_resolve(eth_iface_t * iface, ipv4_addr_t ip_addr, mac_addr_t mac_addr)
{
    //ARP - Address Resolution Protocol
    //reply MAC address when asked who has that IP Address
    /*
    Se pide desarrollar un programa (arp_client.c) basado en la librería librawnet que 
    muestre en pantalla la dirección MAC asociada a la dirección IPv4 que se pasa como 
    parámetro en su línea de comandos, junto con el interfaz que se desea emplear.
    Para ello debe implementar la función “int arp_resolve(eth_iface_t * iface, 
    ipv4_addr_t ip_addr, mac_addr_t mac_addr)” que, dada la dirección IPv4
    “ip_addr”, envíe una petición ARP por el interfaz Ethernet especificado (“iface”) y 
    rellene la dirección “mac_addr” con la respuesta obtenida, o devuelva un valor de error
    distinto de 0 si la respuesta no ha llegado después de 2 segundos. Como todavía no se
    dispone de una dirección IPv4 configurada, la dirección de red origen del mensaje ARP
    debe ser 0.0.0.0.
    El funcionamiento correcto del cliente desarrollado se probará utilizando el servidor 
    ARP implementado por la pila de protocolos TCP/IP estándar de los ordenadores del 
    laboratorio.
    Así, el resultado de ejecutar “arp_client eth0 163.117.144.241” debería ser:
    163.117.144.241 -> 00:10:DC:D9:83:2B
    */

    //Creamos variables auxiliares
    char y[MAC_STR_SIZE];
    //Rellenamos la estructura con los datos correspondientes
    //Comprobamos que la mac introducida se corresponde con una mac 
    //y = mac_str_addr(dest_MAC_addr, arp_header.dest_addr_mac); 

    // Abrimos la interfaz Ethernet

    // Enviamos tramas Ethernet 
    // Inicializamos el temporizador para mantener timeout si se reciben tramas con tipo incorrecto.
    //do
    //{
    //Recibimos tramas Ethernet
    //Comprobamos que tenga un tamaño correcto 
    // Comprobamos si es la trama que estamos buscando 
    //} while (); 
    //Hasta que sea la trama buscada
}

int main(int argc, char* argv[])
{
    //Comprobaciones de nº correcto de argumentos y si son correctos.
    memset(&arp_header_t, 0, sizeof(struct arp_header));
    if(argc != 2)
    {
        fprintf(stderr, "%s\n", "No input arguments.\n");
        printf("Uso: %s <iface> <target_ip>\n", myself);
    printf("       <iface>: Nombre de la interfaz Ethernet\n");
    printf("        <target_ip>: Direccion ip para solicitar su MAC \n");
        exit(-1);
    }
    
    ipv4_addr_t target_ip;
    if (ipv4_str_addr ( argv[2], target_ip ) == -1)
    {
        fprintf(stderr, "%s\n", "Wrong target IP.\n");
        exit(-1);
    }
    char* iface_name = argv[1];
    eth_iface_t* iface_controller = eth_open(iface_name);
    if( eth_open(iface_name) == NULL)
    {
        fprintf(stderr, "%s\n", "Error con la interfaz.\n");
        exit(-1);
    }



    
}