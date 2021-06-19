#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include <string.h>
#include <unistd.h>

t_queue* cola_espera;
t_queue* cola_ready;
t_queue* cola_blocked;

sem_t mutex_cola_espera;
sem_t mutex_cola_ready;
sem_t mutex_cola_blocked;

sem_t activar_planificacion;
sem_t multiprogramacion;
sem_t tripulantes_new;
sem_t tripulantes_ready;
sem_t tripulante_running

bool continuar_planificacion;

void inicializar_planificador(int, char*, int, int, t_log*);

void* fifo();
void* rr();

#endif /* PLANIFICADOR_H_ */
