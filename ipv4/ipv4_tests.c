#include "ipv4.h"
//#include "../ipv4_route_table/ipv4_route_table.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    
    /*  
        ipv4_addr_t subnet_addr; //198.162.1.0
        ipv4_addr_t subnet_mask; //255.255.255.0 -> /24 [0,23]
                        11111111 11111111 11111111 00000000
    */
    
    ipv4_addr_t ip1 = {100,100,100,100};
    ipv4_addr_t ip2 = {100,100,100,0};
    ipv4_addr_t comp = {0,0,0,0};
    for(int i = 0; i < 4; i++){
        comp[i] = ip1[i] && ip2[i];
    }
    
    //int c = 102 & 01;    
    //printf("%d\n", c);
    printf("%d %d %d %d \n", comp[0],comp[1],comp[2],comp[3]);
    
    
    //ipv4_route route;
    
    return 0;
}