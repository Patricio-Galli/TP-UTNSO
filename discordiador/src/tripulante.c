#include "tripulante.h"

t_log* logger;

tripulante* crear_tripulante(int x, int y, int patota, int id, int socket_ram, int socket_mongo, t_log* log) {
	logger = log;
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

	sem_init(nuevo_tripulante->sem_ready, 0, 0);
	sem_init(nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* trip) {
	tripulante* nuevo_tripulante = (tripulante*) trip; //si modifico el interior de ese puntero se modifica de mi lista tambien


	//todo conectarse_con_ram(mongo);
	//todo conectarse_con_disco(ram);

	char* nueva_tarea = NULL;
	//nueva_tarea = solicitar_tarea(); -> solicita la tarea a la ram
	nueva_tarea = "Tarea 1";

	while(nueva_tarea != NULL) {

		sem_wait(mutex_cola_espera);
			queue_push(cola_espera, nuevo_tripulante);
			sem_post(tripulantes_new);
		sem_post(mutex_cola_espera);

		sem_wait(nuevo_tripulante->sem_ready);
			//todo avisar a la ram
			nuevo_tripulante->estado = READY;
			sem_wait(mutex_cola_ready);
				queue_push(cola_ready, nuevo_tripulante);
				sem_post(tripulantes_ready);
			sem_post(mutex_cola_ready);

		sem_wait(nuevo_tripulante->sem_running);
			//todo avisar a la ram
			nuevo_tripulante->estado = RUNNING;
			//ejecutar(nueva_tarea, nuevo_tripulante);
			sem_post(tripulante_running);

		//nueva_tarea = solicitar_tarea(); -> solicita la tarea a la ram
	}
	nuevo_tripulante->estado = EXIT;
	sem_post(multiprogramacion);
	return 0;
}

char* estado_enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}
