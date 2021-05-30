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
#include <utils/utils.h>
#include <semaphore.h>

#include "consola.h"



typedef struct {
    int id_trip;
    int id_patota;
    int estado; //ready, blocked, etc
    pthread_t hilo;
    struct tripulante *sig;
}tripulante;

typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT
}estado_tarea;

typedef struct {
    tripulante* cabeza;
}lista_tripulante;

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

void generar_posiciones(char** parametros, tripulante* nuevo);
tripulante* crear_nodo_trip(int *posiciones);
void agregar_trip_a_lista(tripulante* nuevo_trip, lista_tripulante* lista, int patota_actual);
void* rutina_hilos(int* posiciones);
//void iniciarConsola();
void iniciarConsola();
command_code mapStringToEnum(char *s);

#endif /* DISCORDIADOR_H_ */
