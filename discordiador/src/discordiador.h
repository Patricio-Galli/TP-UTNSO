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
#include <commons/config.h>
#include <utils/utils-client.h>
#include <semaphore.h>
#include "consola.h"

typedef struct {
	int cantidad_tripulantes;
	int* posiciones_tripulantes_x;
	int* posiciones_tripulantes_y;
	int cantidad_tareas;
	char** tareas;
}parametros_iniciar_patota;

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

#define ERROR_CONEXION -1

parametros_iniciar_patota* obtener_parametros(char* buffer_consola);
tripulante* crear_nodo_trip(int *posiciones);
void agregar_trip_a_lista(tripulante* nuevo_trip);
void* rutina_hilos(void* posiciones);
void iniciar_patota(char** input, int* lista_puertos, t_log* logger);
void listar_tripulantes();
int establecer_conexiones(int* ram, int* mongo, t_config* config);


#endif /* DISCORDIADOR_H_ */
