#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "filosofo.h"
#include "garfos.h"
#include "log.h"

#define TEMPO_MIN_MS 50
#define TEMPO_MAX_MS 200

void filosofo_init(Filosofo *f, int id) {
    f->id   = id;
    f->garfo_esq = id;
    f->garfo_dir = (id + 1) % NUM_FILOSOFOS;
    atomic_init(&f->refeicoes, 0);
    f->seed = (unsigned int)(id + 1) * 7919u;
}

static void esperar_aleatorio(Filosofo *f) {
    int ms = TEMPO_MIN_MS + rand_r(&f->seed) % (TEMPO_MAX_MS - TEMPO_MIN_MS + 1);
    usleep(ms * 1000);
}

void *filosofo_vida(void *arg) {
    Filosofo *f = (Filosofo *)arg;
    char msg[64];

    while (atomic_load(&f->refeicoes) < REFEICOES_ALVO) {
        /*Pensando*/
        log_acao(f->id, "comecou a pensar");
        esperar_aleatorio(f);

        /*Tentando pegar*/
        log_acao(f->id, "esta com fome e tenta pegar os garfos");
        pegar_garfos(f);

        /*Comer*/
       int n = atomic_load(&f->refeicoes) + 1;
        snprintf(msg, sizeof msg, "conseguiu os garfos e esta COMENDO (refeicao %d)", n);
        log_acao(f->id, msg);
        esperar_aleatorio(f);
        atomic_store(&f->refeicoes, n);

        /*Devolver os garfos*/
        devolver_garfos(f);
        log_acao(f->id, "devolveu os garfos");
    }

    log_acao(f->id, "terminou todas as refeicoes");
    return NULL;
}