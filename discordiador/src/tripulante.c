#include "tripulante.h"
#include "planificador.h"

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
	bool tareas_disponibles = true, termino_ejecucion;

	if(RAM_ACTIVADA) {
		t_mensaje* mensaje_out;
		mensaje_out = crear_mensaje(TODOOK);
		enviar_mensaje(trip->socket_ram, mensaje_out);
		liberar_mensaje(mensaje_out);
	}

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_out;
		mensaje_out = crear_mensaje(TODOOK);
		enviar_mensaje(trip->socket_mongo, mensaje_out);
		liberar_mensaje(mensaje_out);
	}


	if(RAM_ACTIVADA)
		tarea = solicitar_tarea(trip, &tareas_disponibles);

	if(tareas_disponibles)
		agregar_ready(trip);

	while(tareas_disponibles) {

		if(RAM_ACTIVADA)
			termino_ejecucion = ejecutar(tarea, trip);
		else {
			if(tiene_tareas%2 == 0)
				termino_ejecucion = ejecutar("ESPERAR;3;3;3", trip);
			else
				termino_ejecucion = ejecutar("GENERAR_OXIGENO 10;0;2;4", trip);
		}

		if(termino_ejecucion){
			if(RAM_ACTIVADA)
				tarea = solicitar_tarea(trip, &tareas_disponibles);
			else
				tiene_tareas--;
		}

		if(tiene_tareas == 0)
			tareas_disponibles = false;
	}
	actualizar_estado(trip, EXIT);
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
	sem_wait(&trip->sem_running);

	log_info(logger,"Tripulante %d va a ejecutar tarea %s  -  Posicion actual: %d|%d", trip->id_trip, input, trip->posicion[0], trip->posicion[1]);

	char** buffer = string_split(input, ";");

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_mongo_out;
		t_list* mensaje_mongo_in;

		mensaje_mongo_out = crear_mensaje(EXEC_1);
		agregar_parametro_a_mensaje(mensaje_mongo_out, input, BUFFER);
		enviar_mensaje(trip->socket_mongo, mensaje_mongo_out);
		mensaje_mongo_in = recibir_mensaje(trip->socket_mongo);

		respuesta_OK(mensaje_mongo_in, "Fallo en comunicacion con el MONGO");

		liberar_mensaje(mensaje_mongo_out);
		list_destroy(mensaje_mongo_in);
	}

	moverse(trip, atoi(buffer[1]), atoi(buffer[2]));

	char** comando_tarea = string_split(buffer[0], " ");
	tareas tarea = stringToEnum(comando_tarea[0]);

	if(tarea != ESPERAR && trip->quantum_disponible)
		ejecutar_io(trip, tarea, atoi(comando_tarea[1]));

	bool termino_tarea = esperar(atoi(buffer[3]), trip);

	if(termino_tarea) {
		log_info(logger,"Tripulante %d termino de ejecutar", trip->id_trip);

		if(MONGO_ACTIVADO) {
			t_mensaje* mensaje_mongo_out;
			t_list* mensaje_mongo_in;

			mensaje_mongo_out = crear_mensaje(EXEC_0);

			enviar_mensaje(trip->socket_mongo, mensaje_mongo_out);
			mensaje_mongo_in = recibir_mensaje(trip->socket_mongo);

			respuesta_OK(mensaje_mongo_in, "Fallo en comunicacion con el MONGO");

			liberar_mensaje(mensaje_mongo_out);
			list_destroy(mensaje_mongo_in);
		}
	}

	if(analizar_quantum && !trip->quantum_disponible) {
		log_info(logger,"Tripulante %d se quedo sin quantum", trip->id_trip);

		quitar_running(trip);
		agregar_ready(trip);

		trip->quantum_disponible = true;
		trip->contador_ciclos = 0;
	}
	else
		sem_post(&trip->sem_running); //para que siga ejecutando si no se quedo sin quantum

	return termino_tarea;
}

void moverse(tripulante* trip, int pos_x, int pos_y) {

	while(trip->posicion[0] != pos_x && trip->quantum_disponible) {

		(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	while(trip->posicion[1] != pos_y && trip->quantum_disponible) {

		(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	if(trip->posicion[0] == pos_x && trip->posicion[1] == pos_y)
		log_info(logger,"Tripulante %d llego a %d|%d", trip->id_trip, trip->posicion[0], trip->posicion[1]);
	else
		log_info(logger,"Tripulante %d se quedo en %d|%d en vez de %d|%d", trip->id_trip, trip->posicion[0], trip->posicion[1], pos_x, pos_y);
}

void ejecutar_io(tripulante* trip, tareas tarea, int cantidad) {

	quitar_running(trip);
	agregar_blocked(trip);

	log_info(logger,"Tripulante %d blockeado por IO", trip->id_trip);

	sem_wait(&trip->sem_blocked);

	log_info(logger,"Tripulante %d ejecutando IO", trip->id_trip);

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_mongo_out;
		t_list* mensaje_mongo_in;

		switch(tarea){
			case GENERAR_OXIGENO:
				mensaje_mongo_out = crear_mensaje(GEN_OX);
				break;
			case CONSUMIR_OXIGENO:
				mensaje_mongo_out = crear_mensaje(CON_OX);
				break;
			case GENERAR_COMIDA:
				mensaje_mongo_out = crear_mensaje(GEN_CO);
				break;
			case CONSUMIR_COMIDA:
				mensaje_mongo_out = crear_mensaje(CON_CO);
				break;
			case GENERAR_BASURA:
				mensaje_mongo_out = crear_mensaje(GEN_BA);
				break;
			case DESCARTAR_BASURA:
				mensaje_mongo_out = crear_mensaje(DES_BA);
				break;
			case ESPERAR: break;
		}

		agregar_parametro_a_mensaje(mensaje_mongo_out, (void*)cantidad, ENTERO);
		enviar_mensaje(trip->socket_mongo, mensaje_mongo_out);
		mensaje_mongo_in = recibir_mensaje(trip->socket_mongo);
		respuesta_OK(mensaje_mongo_in, "Fallo al cargar respuesta en el MONGO");

		liberar_mensaje(mensaje_mongo_out);
		list_destroy(mensaje_mongo_in);
	}
	else {
		switch(tarea){
			case GENERAR_OXIGENO:
				log_info(logger,"Tripulante %d: Generando %d de oxigeno",trip->id_trip, cantidad);
				break;
			case CONSUMIR_OXIGENO:
				log_info(logger,"Tripulante %d: Consumiendo %d de oxigeno",trip->id_trip, cantidad);
				break;
			case GENERAR_COMIDA:
				log_info(logger,"Tripulante %d: Generando %d de comida",trip->id_trip, cantidad);
				break;
			case CONSUMIR_COMIDA:
				log_info(logger,"Tripulante %d: Consumiendo %d de comida",trip->id_trip, cantidad);
				break;
			case GENERAR_BASURA:
				log_info(logger,"Tripulante %d: Generando %d de basura",trip->id_trip, cantidad);
				break;
			case DESCARTAR_BASURA:
				log_info(logger,"Tripulante %d: Descartando %d de basura",trip->id_trip, cantidad);
				break;
			case ESPERAR: break;
		}
	}

	sleep(5);
	puede_continuar(trip);
	sem_post(&io_disponible);

	agregar_ready(trip);
	sem_wait(&trip->sem_running);
}

bool esperar(int tiempo, tripulante* trip) {
	while(trip->tiempo_esperado < tiempo && trip->quantum_disponible) {

		trip->tiempo_esperado++;
		log_info(logger,"Tripulante %d: Espero %d de %d",trip->id_trip, trip->tiempo_esperado, tiempo);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	if(trip->tiempo_esperado == tiempo) {
		trip->tiempo_esperado = 0;
		return true;
	}

	return false;
}

void avisar_movimiento(tripulante* trip) {
	if(RAM_ACTIVADA || MONGO_ACTIVADO) {
		t_mensaje* mensaje_out;

		mensaje_out = crear_mensaje(ACTU_T);

		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[0], ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)trip->posicion[1], ENTERO);

		if(RAM_ACTIVADA) {
			t_list* mensaje_ram_in;

			enviar_mensaje(trip->socket_ram, mensaje_out);
			mensaje_ram_in = recibir_mensaje(trip->socket_ram);
			respuesta_OK(mensaje_ram_in, "Fallo en la actualizacion de posicion en RAM");

			list_destroy(mensaje_ram_in);
		}

		if(MONGO_ACTIVADO) {
			t_list* mensaje_mongo_in;

			enviar_mensaje(trip->socket_mongo, mensaje_out);
			mensaje_mongo_in = recibir_mensaje(trip->socket_mongo);
			respuesta_OK(mensaje_mongo_in, "Fallo en la actualizacion de posicion en MONGO");

			list_destroy(mensaje_mongo_in);
		}

	liberar_mensaje(mensaje_out);
	}
}

void actualizar_estado(tripulante* trip, estado estado_trip) {
	trip->estado = estado_trip;

	if(RAM_ACTIVADA) {
		t_mensaje* mensaje_ram_out;
		t_list* mensaje_ram_in;

		mensaje_ram_out = crear_mensaje(ACTU_T);

		agregar_parametro_a_mensaje(mensaje_ram_out, (void*)trip->estado, ENTERO);

		enviar_mensaje(trip->socket_ram, mensaje_ram_out);
		mensaje_ram_in = recibir_mensaje(trip->socket_ram);

		respuesta_OK(mensaje_ram_in, "Fallo en la actualizacion del estado en RAM");
	}
}

bool respuesta_OK(t_list* respuesta, char* mensaje_fallo) {
	if(!validar_mensaje(respuesta, logger)) {
		log_warning(logger, "FALLO EN COMUNICACION");
		return false;
	}
	else if((int)list_get(respuesta, 0) != TODOOK) {
			log_warning(logger, "%s", mensaje_fallo);
			return false;
	}

	return true;
}

void actualizar_quantum(tripulante* trip) {
	trip->contador_ciclos++;

	if(analizar_quantum && trip->contador_ciclos == quantum)
		trip->quantum_disponible = false;
}

void puede_continuar(tripulante* trip) {
	actualizar_quantum(trip);

	if(!continuar_planificacion) {
		log_info(logger,"Tripulante %d pausado", trip->id_trip);
		sem_wait(&trip->sem_running);
		log_info(logger,"Tripulante %d reactivado", trip->id_trip);
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
