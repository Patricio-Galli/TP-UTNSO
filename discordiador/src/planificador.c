#include "planificador.h"

t_log* logger;

int ciclo_CPU;
int quantum;

void inicializar_planificador(int grado_multiprogramacion, char* algoritmo, int ciclo, int q, t_log* log) {
	ciclo_CPU = ciclo;
	quantum = q;
	logger = log;

	queue_create(cola_espera);
	queue_create(cola_ready);
	queue_create(cola_blocked);

	sem_init(mutex_cola_espera, 0, 1);
	sem_init(mutex_cola_ready, 0, 1);
	sem_init(mutex_cola_blocked, 0, 1);

	sem_init(activar_planificacion, 0, 0);
	sem_init(multiprogramacion, 0, grado_multiprogramacion);
	sem_init(tripulantes_new, 0, 0);
	sem_init(tripulantes_ready, 0, 0);
	sem_init(tripulante_running, 0, 1);

	pthread_t hilo_planificador;
	pthread_create(&hilo_planificador, NULL, planificador, algoritmo);
}

void* planificador(void* parametro) {
	sem_wait(activar_planificacion);

	char* algoritmo = (char*) parametro;
	pthread_t hilo_algoritmo;
	continuar_planificacion = true;

	if(!strcmp(algoritmo,"FIFO"))
		pthread_create(&hilo_algoritmo, NULL, fifo, NULL);
	else
		pthread_create(&hilo_algoritmo, NULL, rr, NULL);

	while(continuar_planificacion) {
		sem_wait(tripulantes_new);
		sem_wait(multiprogramacion);

		sem_wait(mutex_cola_espera);
			tripulante* trip_disponible = (tripulante*)queue_pop(cola_espera);
			sem_post(trip_disponible->sem_ready);
		sem_post(mutex_cola_espera);
	}
}

void* fifo() {
	bool continuar = true;
	log_info(logger,"Panificando con algoritmo FIFO con multiprogramacion %d...", multiprogramacion);

	while(continuar) {
		sem_wait(tripulantes_ready);
		sem_wait(tripulante_running);
			sem_wait(mutex_cola_ready);
				tripulante* trip_disponible = (tripulante*)queue_pop(cola_ready);
				sem_post(trip_disponible->sem_running);
			sem_post(mutex_cola_ready);
	}
	log_info(logger,"Planificacion pausada");

	return 0;
}

void* rr() {
	while(continuar_planificacion) {
		log_info(logger,"Panificando con algoritmo RR con multiprogramacion %d y quantum %d ...", multiprogramacion, quantum);
		sleep(ciclo_CPU);
	}

	log_info(logger,"Planificacion pausada");

	return 0;
}

