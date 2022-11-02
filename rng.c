#include <stdio.h>
#include "global_dependencies.h"

int random_port_generator(void) //Funcion de aula global.
{
    /* Inicializar semilla para rand() */
    unsigned int seed = time(NULL);
    srand(seed);
        /* Generar número aleatorio entre 0 y RAND_MAX */
        int dice = rand();
        /* Número entero aleatorio entre 1 y 10 */
        dice = 1024 + (int) (65535.0 * dice / (RAND_MAX + 1.0));
        // if(dice < 1025)
        // {
        //   dice = dice + 1025;//Para que no sea un puerto reservado
        // }

    return dice;
}

int main ( int argc, char * argv[] )
{
    int random_port = random_port_generator();
    printf("Random port -> %d\n", random_port);


    return 0;
}