#include "planificador.h"
bool* salir;

void inicializar_planificador(int grado_multiprocesamiento, char* algoritmo, int ciclo, int q, bool* salida, t_log* log) {
	ciclo_CPU = ciclo;
	quantum = q;
	logger = log;
	salir = salida;

	cola_ready = queue_create();
	//cola_blocked = queue_create();

	tripulantes_running = list_create();

	pthread_mutex_init(&mutex_cola_ready, NULL);
	pthread_mutex_init(&mutex_tripulantes_running, NULL);
	//pthread_mutex_init(&mutex_cola_blocked, NULL);

	sem_init(&activar_planificacion, 0, 0);

	sem_init(&multiprocesamiento, 0, grado_multiprocesamiento);
	sem_init(&tripulantes_ready, 0, 0);

	pthread_t hilo_planificador;
	pthread_create(&hilo_planificador, NULL, planificador, algoritmo);
}

void* planificador(void* algoritmo) {
	sem_wait(&activar_planificacion);
	continuar_planificacion = true;
	tripulantes_trabajando = 0;

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
			list_add(tripulantes_running, trip);
			sem_post(&trip->sem_running);
			tripulantes_trabajando++;
			//log_info(logger,"Tripulante %d running", trip->id_trip);
		pthread_mutex_unlock(&mutex_cola_ready);
	}
	return 0;
}
