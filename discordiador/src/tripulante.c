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
	nuevo_tripulante->contador_ciclos = 0;
	nuevo_tripulante->tiempo_esperado = 0;

	sem_init(&nuevo_tripulante->sem_blocked, 0, 0);
	sem_init(&nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* trip) {
	tripulante* nuevo_tripulante = (tripulante*) trip; //si modifico el interior de ese puntero se modifica de mi lista tambien

	//todo conectarse_con_ram(mongo);
	//todo conectarse_con_disco(ram);

	//nueva_tarea = solicitar_tarea(); //solicita la tarea a la ram

	int tiene_tareas = 4;
	int termino_tarea;

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
		if(tiene_tareas%2 == 0)
			termino_tarea = ejecutar("ESPERAR;3;3;3", nuevo_tripulante);//devuelve 1 si la termino y 0 si no
		else
			termino_tarea = ejecutar("ESPERAR;0;2;4", nuevo_tripulante);

		if(termino_tarea){
			//nueva_tarea = solicitar_tarea();
			tiene_tareas--;
		}
	}
	nuevo_tripulante->estado = EXIT;
	return 0;
}

int ejecutar(char* input, tripulante* trip) {
	log_info(logger,"Tripulante %d running", trip->id_trip);
	trip->estado = RUNNING;

	char** buffer = string_split(input, ";");
	char** comando_tarea = string_split(buffer[0], " ");
	bool tarea_concretada = false;

	tareas tarea = stringToEnum(comando_tarea[0]);

	//todo  avisar al mongo que se empezo a ejecutar esa tarea

	int pos_x = atoi(buffer[1]);
	int pos_y = atoi(buffer[2]);

	while(trip->posicion[0] != pos_x && trip->estado == RUNNING) {
		log_info(logger,"Tripulante %d posicion: %d|%d y busca: %d|%d",trip->id_trip, trip->posicion[0], trip->posicion[1], pos_x, pos_y);
		if(trip->posicion[0] < pos_x)
			trip->posicion[0]++;
		else
			trip->posicion[0]--;
		sleep(ciclo_CPU);
		trip->contador_ciclos++;
		//todo avisar a ram
		//todo avisar a mongo
	}
	while(trip->posicion[1] != pos_y && trip->estado == RUNNING) {
		log_info(logger,"Tripulante %d posicion: %d|%d y busca: %d|%d",trip->id_trip, trip->posicion[0], trip->posicion[1], pos_x, pos_y);
		if(trip->posicion[1] < pos_y)
			trip->posicion[1]++;
		else
			trip->posicion[1]--;
		sleep(ciclo_CPU);
		trip->contador_ciclos++;
		//todo avisar a ram
		//todo avisar a mongo
	}

	if(trip->estado == RUNNING) {
		int cantidad = atoi(buffer[3]);
		switch(tarea){
			case GENERAR_OXIGENO:
				//activar_io
				//generar_oxigeno
				break;
			case CONSUMIR_OXIGENO:
				break;
			case GENERAR_COMIDA:
				break;
			case CONSUMIR_COMIDA:
				break;
			case GENERAR_BASURA:
				break;
			case DESCARTAR_BASURA:
				break;
			case ESPERAR:
				while(trip->tiempo_esperado < cantidad && trip->estado == RUNNING) {
					log_info(logger,"Tripulante %d ESPERANDO %d de %d",trip->id_trip, trip->tiempo_esperado, cantidad);
					trip->contador_ciclos++;
					trip->tiempo_esperado++;
					sleep(ciclo_CPU);
				}
				if(trip->tiempo_esperado == cantidad){
					tarea_concretada = true;
					trip->tiempo_esperado = 0;
				}
				break;
		}
	}

	if(trip->estado != RUNNING){
		trip->contador_ciclos = 0;
	}

	pthread_mutex_lock(&mutex_tripulantes_running);
		quitar(trip, tripulantes_running);
		sem_post(&multiprocesamiento);
		tripulantes_trabajando--;
	pthread_mutex_unlock(&mutex_tripulantes_running);

	if(tarea_concretada) {
		log_info(logger,"Tripulante %d finalizo trabajo", trip->id_trip);//todo avisar al mongo que se termino de ejecutar la tarea
		return 1;
	}
	else {
		log_info(logger,"Tripulante %d pauso trabajo", trip->id_trip);
		return 0;
	}
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

tareas stringToEnum(char *string){
	char* listaDeStrings[]={"GENERAR_OXIGENO", "CONSUMIR_OXIGENO", "GENERAR_COMIDA", "CONSUMIR_COMIDA", "GENERAR_BASURA", "DESCARTAR_BASURA"};

	for(int i=0;i<6;i++){
		if(!strcasecmp(string,listaDeStrings[i])) {
			free(string);
			return i;
		}
	}
	free(string);
	return ESPERAR;
}

char* estado_enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}
