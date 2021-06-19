#include "tripulante.h"

tripulante* crear_tripulante(int x, int y, int patota, int id, int socket_ram, int socket_mongo) {
	tripulante* nuevo_tripulante = malloc(sizeof(tripulante));
	pthread_t nuevo_hilo;

	nuevo_tripulante->id_trip = id;
	nuevo_tripulante->id_patota = patota;
	nuevo_tripulante->estado = NEW;
	nuevo_tripulante->hilo = nuevo_hilo;
	nuevo_tripulante->posicion[0] = x;
	nuevo_tripulante->posicion[1] = y;
	nuevo_tripulante->socket_ram = socket_ram;
	nuevo_tripulante->socket_mongo = socket_mongo;

	sem_init(&nuevo_tripulante->sem_blocked, 0, 0);
	sem_init(&nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* trip) {
	tripulante* nuevo_tripulante = (tripulante*) trip; //si modifico el interior de ese puntero se modifica de mi lista tambien

	//todo conectarse_con_ram(mongo);
	//todo conectarse_con_disco(ram);

	//nueva_tarea = solicitar_tarea(); -> solicita la tarea a la ram

	int tiene_tareas = 4;

	while(tiene_tareas > 0) {

		//todo avisar a la ram
		pthread_mutex_lock(&mutex_cola_ready);
			queue_push(cola_ready, nuevo_tripulante);
			sem_post(&tripulantes_ready);
			nuevo_tripulante->estado = READY;
			log_info(logger,"Tripulante %d ready", nuevo_tripulante->id_trip);
		pthread_mutex_unlock(&mutex_cola_ready);

		sem_wait(&nuevo_tripulante->sem_running);

		//todo avisar a la ram
		nuevo_tripulante->estado = RUNNING;
		log_info(logger,"Tripulante %d running", nuevo_tripulante->id_trip);
		sleep(ciclo_CPU);
		//ejecutar(nueva_tarea, nuevo_tripulante);
		log_info(logger,"Tripulante %d finalizo trabajo", nuevo_tripulante->id_trip);

		pthread_mutex_lock(&mutex_tripulantes_running);
			quitar(nuevo_tripulante, tripulantes_running);
			sem_post(&multiprocesamiento);
		pthread_mutex_unlock(&mutex_tripulantes_running);

		//nueva_tarea = solicitar_tarea(); -> solicita la tarea a la ram
		tiene_tareas--;
	}
	nuevo_tripulante->estado = EXIT;
	return 0;
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

char* estado_enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}
