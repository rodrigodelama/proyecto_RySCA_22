#include "global_dependencies.h"

#include "rawnet.h"
#include "eth.h"
#include "arp.h"
#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "udp.h"
#include "timerms.h"
#include "ripv2.h"
#include "ripv2_route_table.h"

int main(int argc, char ** argcv)
{
  struct layer_udp * my_udp_layer=NULL;
  char * mifichero=argcv[1];
  char * fichero_tabla_renvio=argcv[2];
  char * fichero_rip=argcv[3];

  uint16_t server_port = PUERTO_RIP;
  int i = 0;
  int calc_sig_salto = 0;
  int index_ruta = 0;
  int datos_recibidos = 0;
  timerms_t timeout;
  int index_min;
  timerms_reset(&timeout, TIMEOUT); //esta a 180 segundos

  udp_layer_t * my_udp_layer = udp_open(server_port,mifichero, fichero_tabla_renvio);

  unsigned char buffer_rip[LEN_PAYLOAD_RIP]; //1472 de capacidad
  uint16_t  puerto_cliente;
  ipv4_addr_t ip_router_que_recibimos;
  ripv2_route_table_t * tabla_rip = rip_route_table_create(); //Creamos la tabla de rutas
  rip_route_table_read (fichero_rip, tabla_rip ); //Rellenamos tabla read
  rip_route_table_print(tabla_rip);
  int comprobacion_ruta; //Lo usamos para ponerlo a 1 si nos envian la ruta que queríamos borrar

  while(1)
  {
    long int time_rcv=calcular_menor_temporizador(tabla_rip); //Le metemos la tabla y nos devuelve el menor tiempo y el indice de la ruta de menor tiempo
    printf("El menor tiempo es %ld\n",time_rcv);
    index_min=calcular_indice_menor_temporizador(tabla_rip);

    //RECIBIMOS DURANTE EL MENOR TIEMPO DE LA TABLA

    datos_recibidos = udp_recv(my_udp_layer, &puerto_cliente, ip_router_que_recibimos, buffer_rip, LEN_PAYLOAD_RIP,time_rcv);

    int cuantas_entradas_recibimos = (datos_recibidos - 4)/20;
    struct paquete* paquete_recibido=NULL;
    paquete_recibido = (struct paquete *) buffer_rip;
    ripv2_route_t* ruta_actual = rip_route_table_get(tabla_rip,index_min);
    char  subnet_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(ruta_actual->dir_ip_subred, subnet_str);
    long int tiempo_expiracion = timerms_left(&ruta_actual->temporizador);

    if(datos_recibidos < 0)
    {
      printf("Error en la trama udp recibida\n ");
      return(-1);

    } else if(datos_recibidos == 0) {
      printf("Ha vencido el temporizador, eliminamos la ruta %s\n",subnet_str);
      rip_route_table_remove(tabla_rip,index_min);
      rip_route_table_print(tabla_rip);

    } else if(datos_recibidos > 0 && (tiempo_expiracion == 0)) { //Hemos recibido algo cuando el temporizador de la ruta ha acabado y voy a comprobar si es la que estaba venciendo
      for(i = 0; i < cuantas_entradas_recibimos; i++)
      {
        index_ruta = rip_route_table_find(tabla_rip, paquete_recibido->entry_rip[i].dir_ip_subred, paquete_recibido->entry_rip[i].mascara_subred);
        if(index_min == index_ruta)
        {
          comprobacion_ruta=1;
        }
      }

      if(comprobacion_ruta!=1)
      {
        printf("Eliminamos la ruta %s\n",subnet_str);
        comprobacion_ruta=0;
        rip_route_table_remove(tabla_rip,index_min);
        rip_route_table_print(tabla_rip);
      }
    }

  /*************************** SI NOS ENVIAN RUTAS ****************************/

    if(paquete_recibido->comando == RESPONSE)
    {
      char  ruta_str[IPv4_STR_MAX_LENGTH];
      printf("Cuantas entradas recibimos: %d\n",cuantas_entradas_recibimos);
      for(i=0;i<cuantas_entradas_recibimos;i++)
      {
        ipv4_addr_str(paquete_recibido->entry_rip[i].dir_ip_subred, ruta_str);
        printf("La ruta %d es %s\n ",i,ruta_str);
        index_ruta = rip_route_table_find(tabla_rip,paquete_recibido->entry_rip[i].dir_ip_subred,paquete_recibido->entry_rip[i].mascara_subred);

//                 -------------- NO TENEMOS LA RUTA -----------------------------------------

      if(index_ruta==-1)
      {
        printf("No tenemos la ruta, la creamos en nuestra tabla de rutas\n");
        int metrica_recibida = ntohl(paquete_recibido->entry_rip[i].metrica) +1;
        que haces
        //arreglar esto, NO? 
        //o estoy haciendo el gilipollas
        if(metrica_recibida<16){    //Añadimos la ruta a la tabla
        ripv2_route_t * ruta_nueva = (ripv2_route_t *) malloc(sizeof(struct rip_route));
        memcpy(ruta_nueva-> dir_ip_subred, paquete_recibido->entry_rip[i].dir_ip_subred, IPv4_ADDR_SIZE);
        memcpy(ruta_nueva->mascara_subred, paquete_recibido->entry_rip[i].mascara_subred, IPv4_ADDR_SIZE);
        calc_sig_salto = calcular_siguiente_salto(paquete_recibido->entry_rip[i].siguiente_salto);
        if(calc_sig_salto==0){
            memcpy(ruta_nueva->siguiente_salto, ip_router_que_recibimos, IPv4_ADDR_SIZE);
           }else if(calc_sig_salto==1){
            memcpy(ruta_nueva->siguiente_salto, paquete_recibido->entry_rip[i].siguiente_salto, IPv4_ADDR_SIZE);
           }
           ruta_nueva->metrica = metrica_recibida;
           ruta_nueva->temporizador=timeout; //El tiempo lo ponemos a 180

           rip_route_table_add(tabla_rip, ruta_nueva);
           rip_route_table_print(tabla_rip);
         }else{
           printf("No incluimos la ruta que nos ha llegado porque es inacesible para nosostros\n");
         }

//                              ----------------YA TENEMOS LA RUTA---------------------------
       }else if(index_ruta>-1){
         printf("Nos ha llegado una ruta que ya tenemos, vamos a ver si la añadimos u no\n");

         rip_route_t * ruta_de_nuestra_tabla = NULL;
         rip_route_t * ruta_nueva2 = NULL;
         ruta_de_nuestra_tabla = rip_route_table_get(tabla_rip, index_ruta);
         //if(ruta_de_nuestra_tabla!=NULL){
         if(memcmp(paquete_recibido->entry_rip[i].siguiente_salto,ruta_de_nuestra_tabla->siguiente_salto,IPv4_ADDR_SIZE)==0){

            if(ntohl(paquete_recibido->entry_rip[i].metrica)+1>=16){ //Si es del padre y 16 de metrica
             rip_route_table_remove(tabla_rip,index_ruta);

            }else if(ntohl(paquete_recibido->entry_rip[i].metrica)+1<16){

             memcpy(ruta_nueva2-> dir_ip_subred, ruta_de_nuestra_tabla->dir_ip_subred, IPv4_ADDR_SIZE);
             memcpy(ruta_nueva2->mascara_subred, ruta_de_nuestra_tabla->mascara_subred, IPv4_ADDR_SIZE);
             memcpy(ruta_nueva2->siguiente_salto,ruta_de_nuestra_tabla->siguiente_salto, IPv4_ADDR_SIZE);
             int metrica_nueva = ntohl(paquete_recibido->entry_rip[i].metrica) +1 ;
             ruta_nueva2->metrica = metrica_nueva;
             ruta_nueva2->temporizador=timeout;
            }

        }else{ //Si no viene del padre
         int metrica_nueva = ntohl(paquete_recibido->entry_rip[i].metrica) +1 ;

         if(metrica_nueva < ruta_de_nuestra_tabla->metrica){ //Actualizamos la ruta
           printf("La métrica nueva es menor, actualizamos la ruta\n");

          memcpy(ruta_nueva2-> dir_ip_subred, ruta_de_nuestra_tabla->dir_ip_subred, IPv4_ADDR_SIZE);
          memcpy(ruta_nueva2->mascara_subred, ruta_de_nuestra_tabla->mascara_subred, IPv4_ADDR_SIZE);
          memcpy(ruta_nueva2->siguiente_salto,ruta_de_nuestra_tabla->siguiente_salto, IPv4_ADDR_SIZE);
          ruta_nueva2->metrica = metrica_nueva;
          ruta_nueva2->temporizador=timeout;

          rip_route_table_remove(tabla_rip, index_ruta);
          rip_route_table_add(tabla_rip, ruta_nueva2);
          rip_route_table_print(tabla_rip);

        } //cierra que la metrica sea menor que la que teniamos guardada
      } //Cierra si no viene del padre
    } //Cierra si ya tenemos la ruta
  } //Cierra el for que nos recorre todas las entradas que nos han enviado


    /***********************SI QUIEREN QUE LE ENVIEMOS LA TABLA ********************/

   }else if(paquete_recibido->comando==REQUEST){
      printf("\nRecibimos un request\n");

      int numero_rutas = cuantas_rutas(tabla_rip); //Calcula el numero de rutas
      struct paquete paquete_rip;

      paquete_rip.comando=RESPONSE;
      paquete_rip.version=2;
      paquete_rip.zeros=0x0000;
      rip_route_t * rutas;
      for(i = 0; i<numero_rutas; i++)
      {
        rutas = rip_route_table_get(tabla_rip, i);
        if(rutas!=NULL){
        paquete_rip.entry_rip[i].id_familia = AF_INET; //2
        paquete_rip.entry_rip[i].etiqueta=0x0000;
        memcpy(paquete_rip.entry_rip[i].dir_ip_subred, rutas->dir_ip_subred, IPv4_ADDR_SIZE);
        memcpy(paquete_rip.entry_rip[i].mascara_subred, rutas->mascara_subred, IPv4_ADDR_SIZE);
        memcpy(paquete_rip.entry_rip[i].siguiente_salto, rutas->siguiente_salto, IPv4_ADDR_SIZE);
        paquete_rip.entry_rip[i].metrica=htonl(rutas->metrica);
        }//Si ruta es NULL
      } //Cerramos el for
      int total_longitud=(numero_rutas*20)+4;
      udp_send(my_udp_layer, puerto_cliente, ip_router_que_recibimos, (unsigned char *) &paquete_rip, total_longitud);

   } //Cierra que sea un request
  } //Cerramos el while 1
  udp_close(my_udp_layer);
} //Cerramos el main

/**********************************CONTAMOS CAUNTAS RUTAS HAY EN NUESTRA TABLA***********************************************************/

int cuantas_rutas(rip_route_table_t * tabla_rip )
{
  rip_route_t * ruta=NULL;
  int contador=0;
  int i;
  for(i=0;i<RIP_ROUTE_TABLE_SIZE;i++)
  {
    ruta = rip_route_table_get(tabla_rip, i);
    if(ruta!=NULL)
    {
      contador++;
    }
  }
  return contador;
}

/**********DEVUELVO 0 EN EL CASO NORMAL CUANDO EL SIGUIENTE SALTO ES EL QUE ME HA ENVIADO LAS RUTAS,  Y 1 SI ES EL CASO RARO*********************/

int calcular_siguiente_salto(ipv4_addr_t siguiente_salto)
{
  //memcmp devuelve 0 si son iguales
  int comp = memcmp(IPv4_ZERO_ADDR, siguiente_salto, IPv4_ADDR_SIZE);
  if(comp == 0)
  {
    return 0;
  } else {
    return 1;
  }
}

/******************************DEVUELVE EL MENOR TIEMPO DE LOS TEMORIZADORES DE LAS RUTAS Y EL INDICE DE LA RUTA CON EL MENOR TIEMPO*******************/

int calcular_menor_temporizador(rip_route_table_t * tabla_rip)
{
  int j=0;
  long int time_left;
  long int tiempo_minimo=180000;
  rip_route_t  * ruta_leida= NULL;
  
  for(j=0;j<RIP_ROUTE_TABLE_SIZE;j++)
  {
    ruta_leida= rip_route_table_get(tabla_rip, j);
    if (ruta_leida != NULL)
    {

    time_left= timerms_left(&ruta_leida->temporizador);
    printf("el temporizador  %d :%ld\n",j,time_left );
      if(time_left<tiempo_minimo)
      {
        tiempo_minimo=time_left;
      }
    }
  }
  return tiempo_minimo;
}

int calcular_indice_menor_temporizador(rip_route_table_t * tabla_rip)
{
  int j = 0;
  int index_min;
  long int time_left;
  long int tiempo_minimo = 180000;
  ripv2_route_t* ruta_leida = NULL;

  for(j = 0; j < RIP_ROUTE_TABLE_SIZE; j++)
  {
    ruta_leida= rip_route_table_get(tabla_rip, j);
    if(ruta_leida != NULL)
    {
      time_left= timerms_left(&ruta_leida->temporizador);

      if(time_left<tiempo_minimo)
      {
        tiempo_minimo=time_left;
        index_min=j;

      }
    }
  }
  return index_min;
}
