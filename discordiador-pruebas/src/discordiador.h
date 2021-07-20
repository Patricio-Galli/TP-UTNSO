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
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#include <semaphore.h>
#include "consola.h"

typedef struct {
	int id_trip;
	int id_patota;
	pthread_t* hilo;
	bool muero_rapido;
	bool vivir;
	int socket;
	int px;
	int py;
}t_tripulante;

#define ERROR_CONEXION -1

char* primer_palabra(char* buffer);

void* rutina_hilos(void* posiciones);

void listar_tripulantes();
int establecer_conexiones(int* ram, int* mongo, t_config* config);

t_tripulante* tripulante_de_lista(uint32_t id_patota, uint32_t id_trip);

int posicion_trip(uint32_t id_patota, uint32_t id_trip);
#endif /* DISCORDIADOR_H_ */