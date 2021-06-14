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


#include <commons/string.h> //string_split
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

typedef struct {
	int cantidad_tripulantes;
	int* posicion_x;
	int* posicion_y;
	int cantidad_tareas;
	char** tareas;
}tripulante;

#define ERROR_CONEXION -1

parametros_iniciar_patota* obtener_parametros(char*);

void iniciar_patota(parametros_iniciar_patota*);
int establecer_conexiones(int*, int*, t_config*);
void loggear_parametros(parametros_iniciar_patota*);


#endif /* DISCORDIADOR_H_ */
