#include "arp_client.h"
#include <stdio.h>

#define MAC_STR_SIZE  17
#define HW_TYPE_ETH 1
#define PROT_TYPE_IPV4 0x0800
#define HW_SIZE_MAC_ADDR 6 //6 bytes las dir MAC.
#define PROT_SIZE_IP_ADDR 4// 4 bytes las dir IP.
#define OPCODE_REQUEST 1
#define OPCODE_REPLY 2

//mac_addr_t MAC_BCAST_ADDR = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
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
};


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
    //char y[MAC_STR_SIZE];
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
    struct arp_header arp_header_t; //Creo header de ARP
    memset(&arp_header_t, 0, sizeof(struct arp_header));//Relleno la zona de memoria que guarda nuestra cabecera ARP con 0. 
    arp_header_t.hardware_type= htons(HW_TYPE_ETH);
    arp_header_t.protocol_type = htons(PROT_TYPE_IPV4);
    arp_header_t.hw_size = (uint8_t) HW_SIZE_MAC_ADDR;
    arp_header_t.protocol_size = (uint8_t) PROT_SIZE_IP_ADDR;

        //Montamos el paquete a mandar.
    arp_header_t.opcode = htons(OPCODE_REQUEST);
    eth_getaddr ( &iface, arp_header_t.src_MAC_addr) ;
    ipv4_str_addr("0.0.0.0",arp_header_t.src_IPv4_addr);//Tamaño dirs IP  4 bytes.
    memcpy(arp_header_t.dest_IPv4_addr, ip_addr, 4);
    memcpy(arp_header_t.dest_MAC_addr,  MAC_BCAST_ADDR, MAC_ADDR_SIZE);


    char * name=eth_getname(&iface); //Obtengo el nombre del manejador (dado por parámetro de la función)
    eth_open(&name); //Abro la interfaz
    //Type de ARP = 0x0806
    //Envio ARP Request
    eth_send(&iface,arp_header_t.dest_MAC_addr,0x0806, (unsigned char *) &arp_header_t, sizeof(struct arp_header));
    
    //Recibir el reply
    //variables para almacenar datos de eth_rcv
    mac_addr_t src_addr; 
    unsigned char buffer[ETH_MTU];
    long int timeout = 2000;
    len=eth_recv(&iface,src_addr,0x0806,buffer, ETH_MTU,timeout);
    if (len == -1) { //Si no hay datos
        printf(stderr, "ERROR en eth_recv()\n");
        eth_close(&iface);
        return -1;
    } else if (len == 0) { //Si no hay datos
        fprintf(stderr, "%s: ERROR: No hay respuesta del Servidor Ethernet\n",name);
        eth_close(&iface);
        return -1;
    }
    //Si hay datos, almacenar el mac_address
    mac_addr=src_addr;
    //cerrar interfaz
    eth_close(&iface);
    return 0;
}

int main(int argc, char* argv[])
{

    struct arp_header arp_header_t;
    //Comprobaciones de nº correcto de argumentos y si son correctos.
    memset(&arp_header_t, 0, sizeof(struct arp_header));//Relleno la zona de memoria que guarda nuestra cabecera ARP con 0. 

    if(argc != 3 )
    {
        fprintf(stderr, "%s\n", "No input arguments\n");
        printf("Uso:  <iface> <target_ip>\n");
        printf("        <iface>: Nombre de la interfaz Ethernet\n");
        printf("        <target_ip>: Direccion ip para solicitar su MAC \n");
        exit(-1);
    }
    
    //Potentially we should check for of the network interface (eg: eth0) is valid
    char* iface_name = argv[1];
    eth_iface_t* iface_controller = eth_open(iface_name);
    if( eth_open(iface_name) == NULL)
    {
        fprintf(stderr, "%s\n", "Error con la interfaz\n");
        exit(-1);
    }

    ipv4_addr_t target_ip;
    if (ipv4_str_addr ( argv[2], target_ip ) == -1)
    {
        fprintf(stderr, "\nInvalid target IP Address");
        exit(-1);
    }
    

    //A partir de aqui, los parametros pasados por linea de comandos son correctos.
    //Empezamos a convertir y rellenar los campos de la cabecera ARP.
    //Ya tenemos la IP target convertida en el "if" y el controlador de la interfaz.
    //Podemos meter tal cual los valores de 1 byte (8 bits), que son hw_size y protocol_size.
    //Necesitamos convertir con htons() los de 16 bits, que son hardware_type y protocol_type. Luego el opcode cuando mandemos.

    arp_header_t.hw_size = (uint8_t) HW_SIZE_MAC_ADDR;
    arp_header_t.protocol_size = (uint8_t) PROT_SIZE_IP_ADDR;

    arp_header_t.hardware_type= htons(HW_TYPE_ETH);
    arp_header_t.protocol_type = htons(PROT_TYPE_IPV4);

    //Montamos el paquete a mandar.
    arp_header_t.opcode = htons(OPCODE_REQUEST);
    eth_getaddr ( iface_controller, arp_header_t.src_MAC_addr) ;
    ipv4_str_addr("0.0.0.0",arp_header_t.src_IPv4_addr);//Tamaño dirs IP  4 bytes.
    memcpy(arp_header_t.dest_IPv4_addr, target_ip, 4);
    memcpy(arp_header_t.dest_MAC_addr,  MAC_BCAST_ADDR, MAC_ADDR_SIZE);
    
    //Paquete montado -> Mandamos el paquete:
    //Type de ARP = 0x0806
    
    eth_send ( iface_controller, arp_header_t.dest_MAC_addr, 0x0806, (unsigned char *) &arp_header_t, sizeof(struct arp_header) );

    //arp_resolve(iface_name, target_ip, NULL); //mac_addr should be the thing to recover!!
}