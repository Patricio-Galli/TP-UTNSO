#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/string.h>

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
	sem_t sem_blocked;
	sem_t sem_running;
	int contador_ciclos;
	int tiempo_esperado;
}tripulante;

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT
}estado;

typedef enum {
	GENERAR_OXIGENO,
	CONSUMIR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA,
	ESPERAR
}tareas;

tripulante* crear_tripulante(int, int, int, int, int, int);
void* rutina_tripulante(void*);
char* estado_enumToString(int);

tareas stringToEnum(char *);
void ejecutar(char*, tripulante*);
void moverse(tripulante*, int, int);
void esperar(int, tripulante*);
void quitar(tripulante*, t_list*);

#endif /* TRIPULANTE_H_ */
