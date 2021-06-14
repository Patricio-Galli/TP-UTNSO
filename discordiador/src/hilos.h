#ifndef HILOS_H_
#define HILOS_H_

#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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

tripulante* crear_nodo_trip(int *posiciones);
void agregar_trip_a_lista(tripulante* nuevo_trip, nodo_tripulante *lista_tripulantes);
void* rutina_hilos(void* posiciones);

#endif /* HILOS_H_ */
