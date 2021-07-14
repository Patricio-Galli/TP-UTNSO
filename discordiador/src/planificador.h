#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "tripulante.h"

t_queue* cola_ready;
t_queue* cola_blocked;
t_list* tripulantes_running;
tripulante* trip_block;

pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_tripulantes_running;
pthread_mutex_t mutex_cola_blocked;

sem_t activar_planificacion;
sem_t multiprocesamiento;
sem_t tripulantes_ready;
sem_t io_disponible;
sem_t tripulantes_blocked;

t_log* logger;
int ciclo_CPU;
int quantum;
bool continuar_planificacion;
bool analizar_quantum;

void inicializar_planificador(int, char*, int, int, bool*, t_log*);

void* planificador(void*);
void* planificador_io();
void exit_planificacion();

void agregar_ready(tripulante* trip);
void agregar_running(tripulante* trip);
void agregar_blocked(tripulante* trip);
void quitar_running(tripulante* trip);
void quitar(tripulante*, t_list*);

#endif /* PLANIFICADOR_H_ */
