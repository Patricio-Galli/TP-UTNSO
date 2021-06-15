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
#include "tripulante.h"

typedef struct {
	int cantidad_tripulantes;
	int* posiciones_tripulantes_x;
	int* posiciones_tripulantes_y;
	int cantidad_tareas;
	char** tareas;
}parametros_iniciar_patota;

parametros_iniciar_patota* obtener_parametros(char**);
void loggear_parametros(parametros_iniciar_patota*);

void iniciar_patota(parametros_iniciar_patota*);
void listar_tripulantes();
void expulsar_tripulante(int);

#endif /* DISCORDIADOR_H_ */
