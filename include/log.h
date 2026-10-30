#ifndef LOG_H
#define LOG_H

void log_init(void);
void log_destroy(void);

void log_acao(int id, const char *mensagem);

#endif