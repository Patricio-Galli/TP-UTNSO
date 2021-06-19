#include "planificador.h"

void inicializar_planificador(int grado_multiprocesamiento, char* algoritmo, int ciclo, int q, t_log* log) {
	ciclo_CPU = ciclo;
	quantum = q;
	logger = log;
	continuar_planificacion = true;

	cola_ready = queue_create();
	cola_blocked = queue_create();

	tripulantes_running = list_create();

	pthread_mutex_init(&mutex_cola_ready, NULL);
	pthread_mutex_init(&mutex_tripulantes_running, NULL);
	pthread_mutex_init(&mutex_cola_blocked, NULL);

	sem_init(&activar_planificacion, 0, 0);
	sem_init(&desactivar_planificacion, 0, 0);

	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);
	sem_init(&tripulantes_ready, 0, 0);

	pthread_t hilo_planificador;
	pthread_t hilo_finalizador;

	if(!strcmp(algoritmo,"FIFO"))
		pthread_create(&hilo_planificador, NULL, fifo, NULL);
	else
		pthread_create(&hilo_planificador, NULL, rr, NULL);

	pthread_create(&hilo_finalizador, NULL, finalizador, algoritmo);
}

void* fifo() {
	sem_wait(&activar_planificacion);
	bool continuar = true;
	tripulantes_trabajando = 0;
	log_info(logger,"Panificando con algoritmo FIFO ...");

	while(continuar) {
		sem_wait(&tripulantes_ready);
		sem_wait(&multiprocesamiento);
			if(continuar_planificacion) {
				pthread_mutex_lock(&mutex_cola_ready);
					tripulante* trip = (tripulante*)queue_pop(cola_ready);
					sem_post(&trip->sem_running);
					list_add(tripulantes_running, trip);
					tripulantes_trabajando++;
				pthread_mutex_unlock(&mutex_cola_ready);
			}
			else {
				continuar = false;
				sem_post(&tripulantes_ready);
				sem_post(&multiprocesamiento);
				while(tripulantes_trabajando != 0) {
					tripulante* trip = (tripulante*)list_remove(tripulantes_running, tripulantes_trabajando-1);
					trip->estado = READY;
					tripulantes_trabajando--;
				}
			}
	}
	log_info(logger,"Planificacion pausada");

	return 0;
}

void* rr() {
	sem_wait(&activar_planificacion);

	while(continuar_planificacion) {
		log_info(logger,"Panificando con algoritmo RR quantum %d ...", quantum);
		sleep(ciclo_CPU);
	}

	log_info(logger,"Planificacion pausada");

	return 0;
}

void* finalizador(void* parametro) {
	sem_wait(&desactivar_planificacion);
	continuar_planificacion = false;

	return 0;
}
