#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include <commons/config.h>

#include "consola.h"
#include "tripulante.h"
#include "planificador.h"

typedef struct {
	int cantidad_tripulantes;
	int* posiciones_x;
	int* posiciones_y;
	int cantidad_tareas;
	char** tareas;
}parametros_iniciar_patota;

parametros_iniciar_patota* obtener_parametros(char**);
void liberar_parametros(parametros_iniciar_patota*);
void loggear_parametros(parametros_iniciar_patota*);

void iniciar_patota(parametros_iniciar_patota*);
void listar_tripulantes();
void expulsar_tripulante(int, int);
void iniciar_planificacion();
void pausar_planificacion();

#endif /* DISCORDIADOR_H_ */
