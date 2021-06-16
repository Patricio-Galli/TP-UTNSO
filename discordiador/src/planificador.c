#include "planificador.h"

t_log* logger;

t_list* lista_tripulantes;
int multiprogramacion;
bool planificacion_activada;
int quantum;

t_queue* cola_planificacion;

void planificador(t_list* lista, int multip, char* algoritmo, int q, bool* activada, t_log* log) {
	lista_tripulantes = lista;
	multiprogramacion = multip;
	quantum = q;
	planificacion_activada = *activada;
	logger = log;
	pthread_t hilo_planificador;

	if(!strcmp(algoritmo,"FIFO"))
		pthread_create(&hilo_planificador, NULL, fifo, NULL);
	else
		pthread_create(&hilo_planificador, NULL, rr, NULL);
}

void* fifo() {
	while(planificacion_activada) {
		log_info(logger,"Panificando con algoritmo FIFO con multiprogramacion %d ...", multiprogramacion);
		sleep(5);
	}

	log_info(logger,"Planificacion pausada");

	return 0;
}

void* rr() {
	while(planificacion_activada) {
		log_info(logger,"Panificando con algoritmo RR con multiprogramacion %d y quantum %d ...", multiprogramacion, quantum);
		sleep(5);
	}

	log_info(logger,"Planificacion pausada");

	return 0;
}
/*
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
*/
