#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    int nums = atoi(argv[1]);

    /* Inicializar semilla para rand() */
    unsigned int seed = time(NULL);
    srand(seed);
    int i;
    for (i = 0; i < nums; i++)
    {
        /* Generar número aleatorio entre 0 y RAND_MAX */
        int dice = rand();
        /* Número entero aleatorio entre 1 y 10 */
        dice = 1 + (int) (10.0 * dice / (RAND_MAX + 1.0));
        printf("%i\n", dice);
    }
}
