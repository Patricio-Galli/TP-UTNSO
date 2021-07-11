#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/string.h>

#include <utils/utils-client.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#include "planificador.h"

#define CONEXIONES_ACTIVADAS 0

typedef struct {
	int posicion[2];
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc

	pthread_t hilo;

	int socket_ram;
	int socket_mongo;

	sem_t sem_blocked;
	sem_t sem_running;

	int contador_ciclos;
	int tiempo_esperado;
	bool quantum_disponible;
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
bool ejecutar(char*, tripulante*);
char* solicitar_tarea(tripulante*, bool*);
void avisar_movimiento(tripulante*);
void actualizar_estado(tripulante* trip, estado estado_trip);
void moverse(tripulante*, int, int);
void ejecutar_io(tripulante* trip, char* comando);
bool esperar(int, tripulante*);
void respuesta_OK(t_list* respuesta, char* mensaje_fallo);
void actualizar_quantum(tripulante*);
void puede_continuar(tripulante*);
void quitar(tripulante*, t_list*);

#endif /* TRIPULANTE_H_ */
