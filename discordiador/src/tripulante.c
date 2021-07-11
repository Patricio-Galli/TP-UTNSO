#include "tripulante.h"

tripulante* crear_tripulante(int x, int y, int patota, int id, int socket_ram, int socket_mongo) {
	tripulante* nuevo_tripulante = malloc(sizeof(tripulante));

	nuevo_tripulante->id_trip = id;
	nuevo_tripulante->id_patota = patota;
	nuevo_tripulante->estado = NEW;
	nuevo_tripulante->posicion[0] = x;
	nuevo_tripulante->posicion[1] = y;
	nuevo_tripulante->socket_ram = socket_ram;
	nuevo_tripulante->socket_mongo = socket_mongo;
	nuevo_tripulante->contador_ciclos = 0;
	nuevo_tripulante->tiempo_esperado = 0;
	nuevo_tripulante->quantum_disponible = true;

	sem_init(&nuevo_tripulante->sem_blocked, 0, 0);
	sem_init(&nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_tripulante->hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* t) {
	tripulante* trip = (tripulante*) t; //si modifico el interior de ese puntero se modifica de mi lista tambien
	int tiene_tareas = 4;//todo eliminar al usar las conexiones definitivamente
	char* tarea;
	bool tareas_disponibles = true;

	if(CONEXIONES_ACTIVADAS)
		tarea = solicitar_tarea(trip, &tareas_disponibles);

	while(tareas_disponibles) {
		bool termino_ejecucion;

		//todo avisar a la ram
		trip->estado = READY;

		pthread_mutex_lock(&mutex_cola_ready);
			queue_push(cola_ready, trip);
			sem_post(&tripulantes_ready);
		pthread_mutex_unlock(&mutex_cola_ready);

		sem_wait(&trip->sem_running);

		//todo avisar a la ram
		trip->estado = RUNNING;

		if(CONEXIONES_ACTIVADAS)
			termino_ejecucion = ejecutar(tarea, trip);
		else {
			if(tiene_tareas%2 == 0)
				termino_ejecucion = ejecutar("ESPERAR;3;3;3", trip);
			else
				termino_ejecucion = ejecutar("ESPERAR;0;2;4", trip);
		}

		if(termino_ejecucion){
			if(CONEXIONES_ACTIVADAS)
				tarea = solicitar_tarea(trip, &tareas_disponibles);
			else
				tiene_tareas--;
		}

		if(tiene_tareas == 0)
			tareas_disponibles = false;
	}
	trip->estado = EXIT;
	return 0;
}

char* solicitar_tarea(tripulante* trip, bool* tareas_disponibles) {
	t_mensaje* mensaje_out;
	t_list* mensaje_in;

	mensaje_out = crear_mensaje(NEXT_T);
	enviar_mensaje(trip->socket_ram, mensaje_out);
	mensaje_in = recibir_mensaje(trip->socket_ram);

	if(!validar_mensaje(mensaje_in, logger))
		log_warning(logger, "FALLO EN MENSAJE CON HILO RAM\n");
	else {
		if((int)list_get(mensaje_in, 0) == TASK_T)//significa que hay tareas
			return (char*)list_get(mensaje_in, 1);
		else
			*tareas_disponibles = false;
	}
	return NULL;
}

bool ejecutar(char* input, tripulante* trip) {
	log_info(logger,"Tripulante %d comienza a ejecutar tarea %s  -  Posicion actual: %d|%d  -  Tiempo esperado previamente:%d  -  Quantum disponible: %d", trip->id_trip, input, trip->posicion[0], trip->posicion[1], trip->tiempo_esperado, quantum - trip->contador_ciclos);

	char** buffer = string_split(input, ";");
	char** comando_tarea = string_split(buffer[0], " ");
	t_mensaje* mensaje_mongo_out;
	t_list* mensaje_mongo_in;

	tareas tarea = stringToEnum(comando_tarea[0]);

	if(CONEXIONES_ACTIVADAS) {
		mensaje_mongo_out = crear_mensaje(EXEC_1);

		agregar_parametro_a_mensaje(mensaje_mongo_out, input, BUFFER);

		enviar_mensaje(trip->socket_mongo, mensaje_mongo_out);
		mensaje_mongo_in = recibir_mensaje(trip->socket_mongo);

		respuesta_OK(mensaje_mongo_in, "Fallo en comunicacion con el MONGO");
	}

	moverse(trip, atoi(buffer[1]), atoi(buffer[2]));

	if(trip->quantum_disponible){
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
				break;
		}
	}

	bool termino_ejecucion = esperar(atoi(buffer[3]), trip);

	pthread_mutex_lock(&mutex_tripulantes_running);
		quitar(trip, tripulantes_running);
		tripulantes_trabajando--;
		sem_post(&multiprocesamiento);
	pthread_mutex_unlock(&mutex_tripulantes_running);

	if(termino_ejecucion) {
		log_info(logger,"Tripulante %d termino de ejecutar", trip->id_trip);

		if(CONEXIONES_ACTIVADAS) {
			mensaje_mongo_out = crear_mensaje(EXEC_0);

			enviar_mensaje(trip->socket_mongo, mensaje_mongo_out);
			mensaje_mongo_in = recibir_mensaje(trip->socket_mongo);

			respuesta_OK(mensaje_mongo_in, "Fallo en comunicacion con el MONGO");
		}
	}

	if(analizar_quantum && !trip->quantum_disponible) {
		log_info(logger,"Tripulante %d se quedo sin quantum", trip->id_trip);
		trip->quantum_disponible = true;
		trip->contador_ciclos = 0;
	}

	return termino_ejecucion;
}

void moverse(tripulante* trip, int pos_x, int pos_y) {

	while(trip->posicion[0] != pos_x && trip->quantum_disponible) {
		if(corroborar_quantum(trip)) {
			(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
			avisar_movimiento(trip);
			sleep(ciclo_CPU);
		}

		corroborar_planificacion(trip);
	}

	while(trip->posicion[1] != pos_y && trip->quantum_disponible) {
		if(corroborar_quantum(trip)) {
			(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
			avisar_movimiento(trip);
			sleep(ciclo_CPU);
		}

		corroborar_planificacion(trip);
	}

	if(trip->posicion[0] == pos_x && trip->posicion[1] == pos_y)
		log_info(logger,"Tripulante %d llego a %d|%d", trip->id_trip, trip->posicion[0], trip->posicion[1]);
	else
		log_info(logger,"Tripulante %d se quedo en %d|%d en vez de %d|%d", trip->id_trip, trip->posicion[0], trip->posicion[1], pos_x, pos_y);
}

bool esperar(int tiempo, tripulante* trip) {
	while(trip->tiempo_esperado < tiempo && trip->quantum_disponible) {

		if(corroborar_quantum(trip)) {
			log_info(logger,"Tripulante %d ESPERANDO %d de %d",trip->id_trip, trip->tiempo_esperado, tiempo);
			trip->tiempo_esperado++;
			sleep(ciclo_CPU);
		}

		corroborar_planificacion(trip);
	}

	if(trip->tiempo_esperado == tiempo) {
		trip->tiempo_esperado = 0;
		return true;
	}

	return false;
}

void avisar_movimiento(tripulante* trip) {  //todo avisar a mongo
	t_mensaje* mensaje_out;
	t_list* mensaje_in;

	mensaje_out = crear_mensaje(ACTU_T);

	agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
	agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

	enviar_mensaje(trip->socket_ram, mensaje_out);
	mensaje_in = recibir_mensaje(trip->socket_ram);

	respuesta_OK(mensaje_in, "Fallo en la actualizacion de posicion en RAM");
}

void respuesta_OK(t_list* respuesta, char* mensaje_fallo) {
	if(!validar_mensaje(respuesta, logger))
		log_warning(logger, "FALLO EN COMUNICACION");
	else if((int)list_get(respuesta, 0) != TODOOK)
			log_warning(logger, "%s", mensaje_fallo);
}

bool corroborar_quantum(tripulante* trip) {
	if(analizar_quantum && trip->contador_ciclos == quantum) {
		trip->quantum_disponible = false;
		return false;
	}
	else {
		trip->contador_ciclos++;
		return true;
	}
}

void corroborar_planificacion(tripulante* trip) {
	if(!continuar_planificacion) {
		log_info(logger,"Tripulante %d pausado", trip->id_trip);
		sem_wait(&trip->sem_running);
		log_info(logger,"Tripulante %d reactivado", trip->id_trip);
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
			return i;
		}
	}
	return ESPERAR;
}

char* estado_enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}
