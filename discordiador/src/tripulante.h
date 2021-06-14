#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>

typedef struct {
	int posicion[2];
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc
	pthread_t hilo;
}tripulante;

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT
}estado_tarea;

tripulante* crear_tripulante(int, int, int, int);
void* rutina_hilos(void*);
char* enumToString(int);

#endif /* TRIPULANTE_H_ */
