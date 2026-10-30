#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

static pthread_mutex_t mutex_log;
static struct timespec inicio;

void log_init(void) {
    pthread_mutex_init(&mutex_log, NULL);
    clock_gettime(CLOCK_MONOTONIC, &inicio);
}

void log_destroy(void) {
    pthread_mutex_destroy(&mutex_log);
}

void log_acao(int id, const char *mensagem) {
    struct timespec agora;
    clock_gettime(CLOCK_MONOTONIC, &agora);


    long ms = (agora.tv_sec - inicio.tv_sec) * 1000L + (agora.tv_nsec - inicio.tv_nsec) / 1000000L;

    pthread_mutex_lock(&mutex_log);
    printf("[%5ld ms] Filosofo %d: %s\n", ms, id + 1, mensagem);
    fflush(stdout);
    pthread_mutex_unlock(&mutex_log);

}