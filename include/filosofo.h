#ifndef FILOSOFO_H
#define FILOSOFO_H


#include <stdatomic.h>

#define NUM_FILOSOFOS 5
#define REFEICOES_ALVO 1

typedef struct {
    int         id;
    int         garfo_esq;
    int         garfo_dir;
    atomic_int  refeicoes;
    unsigned int    seed;

} Filosofo;

void filosofo_init(Filosofo *f, int id);
void *filosofo_vida(void *arg);

#endif