#include "planificador.h"
bool* salir;

void inicializar_planificador(int grado_multiprocesamiento, char* algoritmo, int ciclo, int q, bool* salida, t_log* log) {
	ciclo_CPU = ciclo;
	quantum = q;
	logger = log;
	salir = salida;

	cola_ready = queue_create();
	cola_blocked = queue_create();

	cola_emergencia = list_create();
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

		tripulante* trip = quitar_ready();

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

tripulante* quitar_ready() {
	pthread_mutex_lock(&mutex_cola_ready);
		tripulante* trip = (tripulante*)queue_pop(cola_ready);
	pthread_mutex_unlock(&mutex_cola_ready);
	return trip;
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

void emergency_trips_running() {
	while(!list_is_empty(tripulantes_running)) {
		int index = list_size(tripulantes_running);
		tripulante* trip_quitar = (tripulante*)list_get(tripulantes_running, index);
		index--;
		while(index >= 0) {
			tripulante* nuevo_tripulante = (tripulante*)list_get(tripulantes_running, index);

			if(nuevo_tripulante->id_trip < trip_quitar->id_trip || (nuevo_tripulante->id_trip == trip_quitar->id_trip && nuevo_tripulante->id_patota < trip_quitar->id_patota))
				trip_quitar = nuevo_tripulante;

			index--;
		}
		actualizar_estado(trip_quitar, EMERGENCY);
		list_add(cola_emergencia, trip_quitar);
		//todo le digo que deje de trabajar
		list_remove(tripulantes_running, trip_quitar);//todo verificar el caso en el que a medida que los saco de running se sigan metiendo los ready en running
	}
}

void emergency_trips_ready() {
	/*while(!queue_is_empty(tripulantes_ready)) {
		tripulante* trip = quitar_ready();
		list_add(cola_emergencia, trip);

		actualizar_estado(trip, EMERGENCY);
	}*/
}

void* detector_sabotaje(void* socket_mongo) {
	while(!*salir) {
		t_list* mensaje_sabotaje = recibir_mensaje(*(int*)socket_mongo);

		if((int)list_get(mensaje_sabotaje, 0) == SABO_P) {
			int pos_x = (int)list_get(mensaje_sabotaje, 1);
			int pos_y = (int) list_get(mensaje_sabotaje, 2);

			log_info(logger, "Hubo un sabotaje en %d|%d", pos_x, pos_y);

			emergency_trips_running();
			emergency_trips_ready();
			//emergency_blocked

			if(!list_is_empty(cola_emergencia)){
				int index = list_size(tripulantes_running);
				tripulante* resolvedor = (tripulante*)list_get(tripulantes_running, index);
				index--;
				/*
				while(index >= 0) {
					tripulante* posible_resolvedor = (tripulante*)list_get(tripulantes_running, index);

					if(distancia_a(posible_resolvedor, pos_x, pos_y) <= distancia_a(resolvedor, pos_x, pos_y))
						resolvedor = posible_resolvedor;

					index--;
				}*/

				//todo decirle al resolvedor que resuelva el sabotaje
				index = 0;
				do {
					tripulante* trip = (tripulante*)list_get(cola_emergencia, index);
					agregar_ready(trip);
					list_remove(cola_emergencia, trip);
					index++;
				}while(!list_is_empty(cola_emergencia));

			}else
				log_error(logger, "No hay tripulantes en la nave, no se puede resolver el sabotaje");
		}else
			log_warning(logger, "No entendi el mensaje");

		list_destroy(mensaje_sabotaje);
	}
	return 0;
}

void exit_planificacion() {
	//cerrar cola_ready, cola_blocked, tripulantes_running

	//finalizar semaforos mutex_cola_ready, mutex_tripulantes_running,
			//mutex_cola_blocked, activar_planificacion, multiprocesamiento,
			//tripulantes_ready, io_disponible, tripulantes_blocked

	//finalizar hilos hilo_planificador, hilo_planificador_io
}
