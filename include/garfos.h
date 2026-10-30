#ifndef GARFOS_H
#define GARFOS_H

#include "filosofo.h"

void garfos_init(void);
void garfos_destroy(void);
void pegar_garfos(Filosofo *f);
void devolver_garfos(Filosofo *f);

#endif