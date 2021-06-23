#include "planificador.h"
bool* salir;

void inicializar_planificador(int grado_multiprocesamiento, char* algoritmo, int ciclo, int q, bool* salida, t_log* log) {
	ciclo_CPU = ciclo;
	quantum = q;
	logger = log;
	salir = salida;

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
	while(!*salir){
		sem_wait(&activar_planificacion);
		bool continuar = true;
		continuar_planificacion = true;
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
				}
		}
		log_info(logger,"Planificacion pausada :(");
	}
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
	while(!*salir){
		sem_wait(&desactivar_planificacion);
		continuar_planificacion = false;

		pthread_mutex_lock(&mutex_tripulantes_running);
			for(int i = 0; i < tripulantes_trabajando; i++) {
				tripulante* trip = (tripulante*)list_get(tripulantes_running, i);
				trip->estado = READY;
			}
		pthread_mutex_unlock(&mutex_tripulantes_running);
	}
	return 0;
}
