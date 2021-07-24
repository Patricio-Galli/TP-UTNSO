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

t_log* logger;
t_config* config;

t_list* lista_tripulantes;

int id_patota_actual = 1;
int socket_ram = 0, socket_mongo = 0;
char *ip_ram, *ip_mongo;

bool salir;
bool planificacion_inicializada;

parametros_iniciar_patota* obtener_parametros(char** input);
void liberar_parametros(parametros_iniciar_patota* parametros);
void loggear_parametros(parametros_iniciar_patota* parametros);

void iniciar_patota(parametros_iniciar_patota* parametros);
void listar_tripulantes();
void expulsar_tripulante(int tripulante, int patota);
void iniciar_planificacion();
void pausar_planificacion();
void obtener_bitacora(int patota, int tripulante);

#endif /* DISCORDIADOR_H_ */
