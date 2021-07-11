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

sem_t multiprocesamiento;
sem_t tripulantes_ready;

sem_t io_disponible;
sem_t tripulantes_blocked;

int ciclo_CPU;
int quantum;
int tripulantes_trabajando;
bool continuar_planificacion;
bool analizar_quantum;

void inicializar_planificador(int, char*, int, int, bool*, t_log*);

void* planificador(void*);

#endif /* PLANIFICADOR_H_ */
