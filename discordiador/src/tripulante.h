#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include "planificador.h"

typedef struct {
	int posicion[2];
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc
	pthread_t hilo;
	int posicion_lista;
	int socket_ram;
	int socket_mongo;
	sem_t sem_ready;
	sem_t sem_running;
	int contador_ciclos;
}tripulante;

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT
}estado;

tripulante* crear_tripulante(int, int, int, int, int, int, t_log*);
void* rutina_tripulante(void*);
char* estado_enumToString(int);

#endif /* TRIPULANTE_H_ */
