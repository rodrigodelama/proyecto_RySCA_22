#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include "ipv4_route_table.h"

// TESTING ROUTE LOOKUP - WORKS

int ipv4_lookup ( ipv4_route_t * route, ipv4_addr_t addr )
{
  int prefix_length = -1;
  ipv4_addr_t aux; //aux de ipv4 X.X.X.X
  //ipv4_addr_t subnet_mask = route->subnet_mask;
  for(int i = 0; i < 4; i++)
  {
    aux[i] = addr[i] & (route->subnet_mask[i]); //Bit AND con addr y la mask. Se guarda en aux
  }
  if( memcmp(aux, route->subnet_addr, 4) == 0 )
  { //comparo aux con subnet_addr, 4 bytes
    printf("Aux is inside my subnet!!\n");
    prefix_length = 0;
    for(int i = 0; i < 4; i++)
    {
      switch (route->subnet_mask[i])
      { //para cada caso, sumo los bytes correspondientes
        case 255:
          prefix_length += 8;
          break;
        case 254:
          prefix_length += 7;
          break;
        case 252:
          prefix_length += 6;
          break;
        case 248:
          prefix_length += 5;
          break;
        case 240:
          prefix_length += 4;
          break;
        case 224:
          prefix_length += 3;
          break;
        case 192:
          prefix_length += 2;
          break;
        case 128:
          prefix_length += 1;
          break;
        default:
          prefix_length +=0;
          break;
      }
    }
  }else{
    printf("Addr not inside my subnet...\n");
  }
  return prefix_length;
}
int ipv4_convert ( char* str, ipv4_addr_t addr )
{
  int err = -1;

  if (str != NULL)
  {
    unsigned int addr_int[IPv4_ADDR_SIZE];
    int len = sscanf(str, "%d.%d.%d.%d", 
                    &addr_int[0], &addr_int[1], 
                    &addr_int[2], &addr_int[3]);

    if (len == IPv4_ADDR_SIZE)
    {
      int i;
      for (i = 0; i < IPv4_ADDR_SIZE; i++)
      {
        addr[i] = (unsigned char) addr_int[i];
      }
      
      err = 0;
    }
  }
  
  return err;
}

int main(int argc, char* argv[])
{
  ipv4_route_t* my_route = (ipv4_route_t*) (malloc(sizeof(ipv4_route_t)));
  ipv4_addr_t subnet_addr;
  ipv4_addr_t subnet_mask;
  ipv4_addr_t gateway_addr;
  char my_iface[32] = "eth1";
  ipv4_convert("192.100.100.0",subnet_addr);
  ipv4_convert("255.255.255.0",subnet_mask);
  memcpy(my_route->subnet_addr, subnet_addr, sizeof(ipv4_addr_t));
  memcpy(my_route->subnet_mask, subnet_mask, sizeof(ipv4_addr_t));
  memcpy(my_route->gateway_addr, gateway_addr, sizeof(ipv4_addr_t));
  memcpy(my_route->iface,my_iface,sizeof(my_iface));
  ipv4_addr_t dest_addr;
  ipv4_convert("192.100.200.101",dest_addr);
  int lenght = ipv4_lookup (my_route, dest_addr);
  printf("Length: %d\n",lenght);


  return 0;
}
