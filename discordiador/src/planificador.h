#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "tripulante.h"

t_log* logger;

t_queue* cola_ready;
t_queue* cola_blocked;

t_list* tripulantes_running;

pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_tripulantes_running;
pthread_mutex_t mutex_cola_blocked;

sem_t activar_planificacion;
sem_t desactivar_planificacion;

sem_t multiprocesamiento;
sem_t tripulantes_ready;

int ciclo_CPU;
int quantum;
bool continuar_planificacion;
int tripulantes_trabajando;

void inicializar_planificador(int, char*, int, int, t_log*);

void* fifo();
void* rr();
void* finalizador(void*);

#endif /* PLANIFICADOR_H_ */
