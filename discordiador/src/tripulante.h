#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>
//#include <stdio.h>

#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/log.h>

typedef struct {
	int posicion[2];
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc
	pthread_t hilo;
	int posicion_lista;
	int socket_ram;
	int socket_mongo;
	//sem_t es_mi_turno
}tripulante;

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT
}estado_tarea;

tripulante* crear_tripulante(int, int, int, int, int, int, t_log*);
void* rutina_tripulante(void*);
char* estado_enumToString(int);

#endif /* TRIPULANTE_H_ */
