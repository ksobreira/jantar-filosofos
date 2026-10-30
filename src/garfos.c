#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "filosofo.h"
#include "garfos.h"
#include "log.h"

static pthread_mutex_t garfos[NUM_FILOSOFOS];

#define ATRASO_ENTRE_GARFOS_US 100000

void garfos_init(void) {
    for (int i = 0; i < NUM_FILOSOFOS; i++)
        pthread_mutex_init(&garfos[i], NULL);
}

void garfos_destroy(void) {
    for (int i = 0; i < NUM_FILOSOFOS; i++)
    pthread_mutex_destroy(&garfos[i]);
}

static void pegar_um(Filosofo *f, int garfo) {
    pthread_mutex_lock(&garfos[garfo]);
    char msg[64];
    snprintf(msg, sizeof msg, "pegou o garfo %d", garfo);
    log_acao(f->id,msg);
}

void pegar_garfos(Filosofo *f) {
    int primeiro = (f->garfo_esq < f->garfo_dir) ? f->garfo_esq : f->garfo_dir;
    int segundo  = (f->garfo_esq < f->garfo_dir) ? f->garfo_dir : f->garfo_esq;

    pegar_um(f, primeiro);
    usleep(ATRASO_ENTRE_GARFOS_US);
    pegar_um(f, segundo);
}

void devolver_garfos(Filosofo *f) {
    pthread_mutex_unlock(&garfos[f->garfo_esq]);
    pthread_mutex_unlock(&garfos[f->garfo_dir]);
}


