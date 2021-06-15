#include "planificador.h"

t_log* logger;
int multiprogramacion;
t_list* lista_tripulantes;
bool planificacion_activada;
int quantum;

t_queue* cola_planificacion;



void iniciar_FIFO(int multip, t_list* lista, bool* activada, t_log* log) {
	log_info(logger,"Iniciando FIFO con multiprogramacion %d", multip);

	logger = log;
	multiprogramacion = multip;
	lista_tripulantes = lista;
	planificacion_activada = *activada;
	cola_planificacion = queue_create();

	//while(*planificacion_activada)
}

void iniciar_RR(int multip, int q, t_list* lista, bool* activada, t_log* log) {
	log_info(logger,"Iniciando RR con multiprogramacion %d y quantum %d", multip, q);

	logger = log;
	multiprogramacion = multip;
	lista_tripulantes = lista;
	planificacion_activada = *activada;
	quantum = q;
	cola_planificacion = queue_create();
}
