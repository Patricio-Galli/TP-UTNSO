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
	nuevo_tripulante->quantum_disponible = true;

	sem_init(&nuevo_tripulante->sem_blocked, 0, 0);
	sem_init(&nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* trip) {
	tripulante* nuevo_tripulante = (tripulante*) trip; //si modifico el interior de ese puntero se modifica de mi lista tambien
	int tiene_tareas = 4;
	char* tarea;
	bool tareas_disponibles = true;

	if(CONEXIONES_ACTIVADAS)
		tarea = solicitar_tarea(nuevo_tripulante, &tareas_disponibles);

	while(tareas_disponibles) {
		bool termino_ejecucion;

		//todo avisar a la ram
		pthread_mutex_lock(&mutex_cola_ready);
			queue_push(cola_ready, nuevo_tripulante);
			sem_post(&tripulantes_ready);
			nuevo_tripulante->estado = READY;
			//log_info(logger,"Tripulante %d ready", nuevo_tripulante->id_trip);
		pthread_mutex_unlock(&mutex_cola_ready);

		sem_wait(&nuevo_tripulante->sem_running);

		//todo avisar a la ram

		if(CONEXIONES_ACTIVADAS)
			termino_ejecucion = ejecutar(tarea, nuevo_tripulante);//devuelve true si la termino y false si no
		else {
			if(tiene_tareas%2 == 0)
				termino_ejecucion = ejecutar("ESPERAR;3;3;3", nuevo_tripulante);//devuelve true si la termino y false si no
			else
				termino_ejecucion = ejecutar("ESPERAR;0;2;4", nuevo_tripulante);
		}

		if(termino_ejecucion){
			if(CONEXIONES_ACTIVADAS)
				tarea = solicitar_tarea(nuevo_tripulante, &tareas_disponibles);
			else
				tiene_tareas--;
		}

		if(tiene_tareas == 0)
			tareas_disponibles = false;
	}
	nuevo_tripulante->estado = EXIT;
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

	trip->estado = RUNNING;

	char** buffer = string_split(input, ";");
	char** comando_tarea = string_split(buffer[0], " ");

	tareas tarea = stringToEnum(comando_tarea[0]);

	//todo  avisar al mongo que se empezo a ejecutar esa tarea

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

	esperar(atoi(buffer[3]), trip);

	pthread_mutex_lock(&mutex_tripulantes_running);
		quitar(trip, tripulantes_running);
		tripulantes_trabajando--;
		sem_post(&multiprocesamiento);
	pthread_mutex_unlock(&mutex_tripulantes_running);
	if(analizar_quantum) {
		if(trip->quantum_disponible) {
			log_info(logger,"Tripulante %d termino de ejecutar  -  Quantum disponible %d", trip->id_trip, quantum - trip->contador_ciclos);
			return true;
		}
		else {
			log_info(logger,"Tripulante %d se quedo sin quantum", trip->id_trip);
			trip->quantum_disponible = true;
			trip->contador_ciclos = 0;
			return false;
		}
	}else {
		log_info(logger,"Tripulante %d termino de ejecutar", trip->id_trip);
		return true;
	}
}

void moverse(tripulante* trip, int pos_x, int pos_y) {
	bool completo_movimiento = true;

	while(trip->posicion[0] != pos_x && trip->quantum_disponible) {

		if(corroborar_quantum(trip)) {
			(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
			avisar_movimiento(trip); //todo avisar a mongo
			sleep(ciclo_CPU);
		}
		else
			completo_movimiento = false;

		corroborar_planificacion(trip);
	}

	if(completo_movimiento)
		log_info(logger,"Tripulante %d llego a x", trip->id_trip);
	else
		log_info(logger,"Tripulante %d se quedo en %d de x en vez de llegar a %d por fin de quantum", trip->id_trip, trip->posicion[0], pos_x);

	while(trip->posicion[1] != pos_y && trip->quantum_disponible) {
		if(corroborar_quantum(trip)) {
			(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
			avisar_movimiento(trip); //todo avisar a mongo
			sleep(ciclo_CPU);
		}
		else
			completo_movimiento = false;

		corroborar_planificacion(trip);
	}

	if(completo_movimiento)
		log_info(logger,"Tripulante %d llego a y", trip->id_trip);
	else
		log_info(logger,"Tripulante %d se quedo en %d de y en vez de llegar a %d por fin de quantum", trip->id_trip, trip->posicion[1], pos_y);
}

void esperar(int tiempo, tripulante* trip) {
	bool completo_espera = true;

	while(trip->tiempo_esperado < tiempo && trip->quantum_disponible) {
		if(corroborar_quantum(trip)) {
			log_info(logger,"Tripulante %d ESPERANDO %d de %d",trip->id_trip, trip->tiempo_esperado, tiempo);
			trip->tiempo_esperado++;
			sleep(ciclo_CPU);
		}
		else
			completo_espera = false;

		corroborar_planificacion(trip);
	}

	if(completo_espera)
		trip->tiempo_esperado = 0;
}

void avisar_movimiento(tripulante* trip) {
	t_mensaje* mensaje_out;
	t_list* mensaje_in;

	mensaje_out = crear_mensaje(ACTU_T);

	agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
	agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

	enviar_mensaje(trip->socket_ram, mensaje_out);
	mensaje_in = recibir_mensaje(trip->socket_ram);

	if(!validar_mensaje(mensaje_in, logger))
		log_warning(logger, "FALLO EN MENSAJE CON HILO RAM");
	else
		if((int)list_get(mensaje_in, 0) != TODOOK)
			log_warning(logger, "FALLO actualizacion de posicion en RAM");
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
