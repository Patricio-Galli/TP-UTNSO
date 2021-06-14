#ifndef HILOS_H_
#define HILOS_H_

#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

typedef struct {
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc
	pthread_t hilo;
}tripulante;

typedef struct nodo_tripulante{
    tripulante data;
    struct nodo_tripulante *sig;
}nodo_tripulante;

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT
}estado_tarea;

tripulante* crear_nodo_trip(int*);
void agregar_trip_a_lista(tripulante*, nodo_tripulante*);
void* rutina_hilos(void*);

#endif /* HILOS_H_ */
