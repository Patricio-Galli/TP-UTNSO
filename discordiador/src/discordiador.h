/*
 * discordiador.h
 *
 *  Created on: 13 may. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// bibliotecas para hilos
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utils/utils.h>
#include <semaphore.h>

#include "consola.h"


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

typedef enum{
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXIT_DISCORDIADOR,
	ERROR
}command_code;

tripulante* crear_nodo_trip(int *posiciones);
void agregar_trip_a_lista(tripulante* nuevo_trip);
void* rutina_hilos(void* posiciones);
command_code mapStringToEnum(char *s);
void iniciar_patota(char** input, t_log* logger);
void listar_tripulantes();


#endif /* DISCORDIADOR_H_ */
