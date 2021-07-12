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

	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);
	sem_init(&tripulantes_ready, 0, 0);

	sem_init(&io_disponible, 0, 1);
	sem_init(&tripulantes_blocked, 0, 0);

	pthread_t hilo_planificador;
	pthread_t hilo_planificador_io;

	pthread_create(&hilo_planificador, NULL, planificador, algoritmo);
	pthread_create(&hilo_planificador_io, NULL, planificador_io, NULL);
}

void* planificador(void* algoritmo) {
	sem_wait(&activar_planificacion);
	continuar_planificacion = true;

	if(!strcmp(algoritmo,"FIFO")) {
		log_info(logger,"Panificando con algoritmo FIFO ...");
		analizar_quantum = false;
	}
	else {
		log_info(logger,"Panificando con algoritmo RR quantum %d ...", quantum);
		analizar_quantum = true;
	}

	while(!*salir) {
		sem_wait(&tripulantes_ready);
		sem_wait(&multiprocesamiento);

		pthread_mutex_lock(&mutex_cola_ready);
			tripulante* trip = (tripulante*)queue_pop(cola_ready);
		pthread_mutex_unlock(&mutex_cola_ready);

		agregar_running(trip);
	}
	return 0;
}

void* planificador_io() {
	while(!*salir) {
		sem_wait(&tripulantes_blocked);
		sem_wait(&io_disponible);

		pthread_mutex_lock(&mutex_cola_blocked);
			tripulante* trip = (tripulante*)queue_pop(cola_blocked);
			trip_block = trip;
		pthread_mutex_unlock(&mutex_cola_blocked);

		sem_post(&trip->sem_blocked);
	}

	return 0;
}

void agregar_ready(tripulante* trip) {
	actualizar_estado(trip, READY);

	pthread_mutex_lock(&mutex_cola_ready);
		queue_push(cola_ready, trip);
		sem_post(&tripulantes_ready);
	pthread_mutex_unlock(&mutex_cola_ready);
}

void agregar_running(tripulante* trip) {
	actualizar_estado(trip, RUNNING);

	pthread_mutex_lock(&mutex_tripulantes_running);
		list_add(tripulantes_running, trip);
		sem_post(&trip->sem_running);
	pthread_mutex_unlock(&mutex_tripulantes_running);
}

void agregar_blocked(tripulante* trip) {
	actualizar_estado(trip, BLOCKED);

	pthread_mutex_lock(&mutex_cola_blocked);
		queue_push(cola_blocked, trip);
		sem_post(&tripulantes_blocked);
	pthread_mutex_unlock(&mutex_cola_blocked);
}

void quitar_running(tripulante* trip) {
	pthread_mutex_lock(&mutex_tripulantes_running);
		quitar(trip, tripulantes_running);
		sem_post(&multiprocesamiento);
	pthread_mutex_unlock(&mutex_tripulantes_running);
}

void quitar(tripulante* trip, t_list* list) {
	int index = 0;
	bool continuar = true;

	while(continuar) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(list, index);

		if(nuevo_tripulante->id_trip == trip->id_trip && nuevo_tripulante->id_patota == trip->id_patota) {
			continuar = false;
			list_remove(list, index);
		}
		index++;
	}
}
