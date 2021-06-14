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

#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/utils-client.h>


#include "consola.h"
#include "hilos.h"

typedef struct {
	int cantidad_tripulantes;
	int* posiciones_tripulantes_x;
	int* posiciones_tripulantes_y;
	int cantidad_tareas;
	char** tareas;
}parametros_iniciar_patota;

#define ERROR_CONEXION -1

parametros_iniciar_patota* obtener_parametros(char* buffer_consola);

void iniciar_patota(char** input, int* lista_puertos, t_log* logger);
//void listar_tripulantes();
int establecer_conexiones(int* ram, int* mongo, t_config* config);


#endif /* DISCORDIADOR_H_ */
