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
	sem_init(&finalizo_sabotaje, 0, 0);

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

		if(hay_sabotaje)
			sem_wait(&finalizo_sabotaje);

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

void agregar_emergencia(tripulante* trip) {
	actualizar_estado(trip, EMERGENCY);
	list_add(cola_emergencia, trip);
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
		int index = list_size(tripulantes_running)-1;

		tripulante* trip_quitar = (tripulante*)list_get(tripulantes_running, index);
		int indice_trip_quitar = index;
		index--;

		while(index >= 0) {
			tripulante* nuevo_tripulante = (tripulante*)list_get(tripulantes_running, index);

			if(nuevo_tripulante->id_trip < trip_quitar->id_trip || (nuevo_tripulante->id_trip == trip_quitar->id_trip && nuevo_tripulante->id_patota < trip_quitar->id_patota)) {
				trip_quitar = nuevo_tripulante;
				indice_trip_quitar = index;
			}

			index--;
		}
		log_info(logger, "Quitando de running al trip %d", trip_quitar->id_trip);
		trip_quitar->continuar = false;
		agregar_emergencia(trip_quitar);
		sem_wait(&trip_quitar->sem_blocked);
		list_remove(tripulantes_running, indice_trip_quitar);
	}
}

void emergency_trips_ready() {
	log_info(logger, "Quitando tripulantes de ready");
	while(!queue_is_empty(cola_ready)) {
		tripulante* trip = quitar_ready();
		log_info(logger, "Quitando de ready al trip %d", trip->id_trip);
		agregar_emergencia(trip);
	}
}

void* detector_sabotaje(void* socket_mongo) {
	hay_sabotaje = false;
	int socket_sabotajes = *(int*)socket_mongo;
	while(!*salir) {
		t_list* mensaje_sabotaje = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_sabotaje, 0) == SABO_P) {
			hay_sabotaje = true;

			int pos_x = (int)list_get(mensaje_sabotaje, 1);
			int pos_y = (int) list_get(mensaje_sabotaje, 2);

			log_warning(logger, "Hubo un sabotaje en %d|%d", pos_x, pos_y);

			emergency_trips_running();
			emergency_trips_ready();
			//emergency_blocked -> se hace en ejecutar_io()

			if(!list_is_empty(cola_emergencia)){
				int index = list_size(cola_emergencia)-1;
				tripulante* resolvedor = (tripulante*)list_get(cola_emergencia, index);
				int indice_resolvedor = index;
				index--;

				while(index >= 0) {
					tripulante* posible_resolvedor = (tripulante*)list_get(cola_emergencia, index);

					if(distancia_a(posible_resolvedor, pos_x, pos_y) <= distancia_a(resolvedor, pos_x, pos_y)) {
						resolvedor = posible_resolvedor;
						indice_resolvedor = index;
					}

					index--;
				}

				log_info(logger, "Resolvedor %d", resolvedor->id_trip);

				resolver_sabotaje(resolvedor, pos_x, pos_y, socket_sabotajes);

				list_remove(cola_emergencia, indice_resolvedor);
				list_add(cola_emergencia, resolvedor);

				log_info(logger, "Resolvedor mandado al final de la cola");

				do {
					tripulante* trip = (tripulante*)list_get(cola_emergencia, 0);
					log_info(logger, "Tripulante %d en ready nuevamente", trip->id_trip);
					trip->continuar = true;
					agregar_ready(trip);
					list_remove(cola_emergencia, 0);
				}while(!list_is_empty(cola_emergencia));
				//hasta aca funciona bien
				hay_sabotaje = false;
				sem_post(&finalizo_sabotaje);

			}else
				log_error(logger, "No hay tripulantes en la nave, no se puede resolver el sabotaje");
		}else
			log_warning(logger, "No entendi el mensaje");

		list_destroy(mensaje_sabotaje);
	}
	return 0;
}

void resolver_sabotaje(tripulante* trip, int pos_x, int pos_y, int socket_sabotajes) {
	bool estado_q = trip->quantum_disponible;
	bool estado_continuar = trip->continuar;
	t_mensaje* mensaje_ini = crear_mensaje(SABO_I);
	log_info(logger, "Enviando mensaje inicio");
	enviar_y_verificar(mensaje_ini, socket_sabotajes, "Fallo al iniciar resolucion del sabotaje");

	trip->quantum_disponible = true;
	trip->continuar = true;

	moverse(trip, pos_x, pos_y);

	trip->continuar = estado_continuar;
	trip->quantum_disponible = estado_q;

	log_info(logger, "Enviando mensaje fin, posicion %d|%d", trip->posicion[0], trip->posicion[1]);
	t_mensaje* mensaje_fin = crear_mensaje(SABO_F);
	enviar_y_verificar(mensaje_fin, socket_sabotajes, "Fallo al finalizar resolucion del sabotaje");
}

int distancia_a(tripulante* trip, int pos_x, int pos_y) {
	//return (int) sqrt(pow(pos_x-trip->posicion[0], 2) + pow(pos_y-trip->posicion[1], 2));
	int distancia, resta_x = pos_x-trip->posicion[0], resta_y = pos_y-trip->posicion[1];

	if(resta_x >= 0)
		distancia = resta_x;
	else
		distancia = -resta_x;

	if(resta_y >= 0)
		distancia += resta_y;
	else
		distancia += -resta_y;

	return distancia;

}

void exit_planificacion() {
	//cerrar cola_ready, cola_blocked, tripulantes_running

	//finalizar semaforos mutex_cola_ready, mutex_tripulantes_running,
			//mutex_cola_blocked, activar_planificacion, multiprocesamiento,
			//tripulantes_ready, io_disponible, tripulantes_blocked

	//finalizar hilos hilo_planificador, hilo_planificador_io
}
