/*
 * discordiador.h
 *
 *  Created on: 13 may. 2021
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
// bibliotecas para hilos
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "string1.h"
#include <semaphore.h>

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
    ERROR_CONSOLA,
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
}command_code;

void generar_posiciones(char** parametros, tripulante* nuevo);
tripulante* crear_nodo_trip(int* posiciones);
tripulante* _crear_nodo_trip(int posiciones[2]);
void agregar_trip_a_lista(tripulante* nuevo_trip, lista_tripulante* lista, int patota_actual);
void* rutina_hilos(int* posiciones);
void* _rutina_hilos(int posiciones[2]);
//void iniciarConsola();
void iniciarConsola();
command_code mapStringToEnum(char *s);

#endif /* CONSOLA_H_ */
