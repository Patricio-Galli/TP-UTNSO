#include "planificador.h"

t_log* logger;

int multiprogramacion;
int ciclo_CPU;
bool planificacion_activada;
int quantum;

t_queue* cola_planificacion;

void planificador(int multip, char* algoritmo, int ciclo, int q, bool* activada, t_log* log) {
	multiprogramacion = multip;
	ciclo_CPU = ciclo;
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
		sleep(ciclo_CPU);
	}

	log_info(logger,"Planificacion pausada");

	return 0;
}

void* rr() {
	while(planificacion_activada) {
		log_info(logger,"Panificando con algoritmo RR con multiprogramacion %d y quantum %d ...", multiprogramacion, quantum);
		sleep(ciclo_CPU);
	}

	log_info(logger,"Planificacion pausada");

	return 0;
}
