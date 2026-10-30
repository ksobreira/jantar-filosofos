#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "filosofo.h"
#include "garfos.h"
#include "log.h"

#define SEGUNDOS_SEM_PROGRESSO 5 

static int total_refeicoes(Filosofo fil[])
{
    int total = 0;
    for (int i = 0; i < NUM_FILOSOFOS; i++)
        total += atomic_load(&fil[i].refeicoes);
    return total;
}

int main(void)
{
    Filosofo  fil[NUM_FILOSOFOS];
    pthread_t threads[NUM_FILOSOFOS];

    log_init();
    garfos_init();

    printf("Estrategia: HIERARQUIA DE RECURSOS\n");

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        filosofo_init(&fil[i], i);
        pthread_create(&threads[i], NULL, filosofo_vida, &fil[i]);
    }

    int meta = NUM_FILOSOFOS * REFEICOES_ALVO;
    int ultimo = -1, parado = 0;

    while (total_refeicoes(fil) < meta) {
        sleep(1);
        int atual = total_refeicoes(fil);
        parado = (atual == ultimo) ? parado + 1 : 0;
        ultimo = atual;

        if (parado >= SEGUNDOS_SEM_PROGRESSO) {
            printf("\nDEADLOCK DETECTADO: ninguem come ha %d s "
                   "(%d de %d refeicoes feitas)\n", parado, atual, meta);
            for (int i = 0; i < NUM_FILOSOFOS; i++)
                printf("Filosofo %d: %d refeicoes\n", i + 1, atomic_load(&fil[i].refeicoes));
            exit(1);  
        }
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++)
        pthread_join(threads[i], NULL);

    printf("\nResumo (refeicoes por filosofo)\n");
    for (int i = 0; i < NUM_FILOSOFOS; i++)
        printf("Filosofo %d: %d refeicoes\n", i + 1, atomic_load(&fil[i].refeicoes));

    garfos_destroy();
    log_destroy();
    return 0;
}