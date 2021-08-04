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
	nuevo_tripulante->continuar = true;

	sem_init(&nuevo_tripulante->sem_blocked, 0, 0);
	sem_init(&nuevo_tripulante->sem_running, 0, 0);

	pthread_create(&nuevo_tripulante->hilo, NULL, rutina_tripulante, nuevo_tripulante);

	return nuevo_tripulante;
}

void* rutina_tripulante(void* t) {
	tripulante* trip = (tripulante*) t;
	char* tarea;
	bool termino_ejecucion;

	tarea = solicitar_tarea(trip);

	if(strcmp(tarea, "no_task") != 0)
		agregar_ready(trip);

	while(strcmp(tarea, "no_task") != 0 && trip->estado != EXIT) {
		termino_ejecucion = ejecutar(tarea, trip);

		if(termino_ejecucion)
			tarea = solicitar_tarea(trip);
	}

	if(trip->estado != EXIT) {
		log_warning(logger,"Tripulante %d finalizando trabajo", trip->id_trip);

		actualizar_estado(trip, EXIT);
		quitar_running(trip);
	}

	sem_destroy(&trip->sem_blocked);
	sem_destroy(&trip->sem_running);

	return 0;
}

bool ejecutar(char* input, tripulante* trip) {

	if(trip->estado == EMERGENCY) {
		log_info(logger,"Tripulante %d bloqueado emergency", trip->id_trip);
		sem_post(&trip->sem_blocked);
		sem_post(&multiprocesamiento);
	}

	if(trip->estado != RUNNING)
		sem_wait(&trip->sem_running);

	log_info(logger,"Tripulante %d va a ejecutar tarea %s", trip->id_trip, input);

	char** buffer = string_split(input, ";");

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_out = crear_mensaje(EXEC_1);
		agregar_parametro_a_mensaje(mensaje_out, (void*)input, BUFFER);
		enviar_y_verificar(mensaje_out, trip->socket_mongo, "Fallo en comunicacion con el mongo");
	}

	bool completo_movimiento = moverse(trip, atoi(buffer[1]), atoi(buffer[2]));

	if(trip->quantum_disponible && trip->continuar && completo_movimiento) {
		char** comando_tarea = string_split(buffer[0], " ");
		tareas tarea = stringToEnum(comando_tarea[0]);

		if(tarea != ESPERAR)
			ejecutar_io(trip, tarea, atoi(comando_tarea[1]));

		liberar_input(comando_tarea);
	}

	bool termino_tarea = esperar(atoi(buffer[3]), trip);

	if(termino_tarea) {
		log_info(logger,"Tripulante %d termino de ejecutar", trip->id_trip);

		if(MONGO_ACTIVADO)
			enviar_y_verificar(crear_mensaje(EXEC_0), trip->socket_mongo, "Fallo en comunicacion con el mongo");
	}

	if(!trip->quantum_disponible && trip->continuar) {
		log_info(logger,"Tripulante %d se quedo sin quantum", trip->id_trip);

		quitar_running(trip);
		agregar_ready(trip);

		trip->quantum_disponible = true;
		trip->contador_ciclos = 0;
	}

	liberar_input(buffer);

	return termino_tarea;
}

void moverse(tripulante* trip, int pos_x, int pos_y) {

	while(trip->posicion[0] != pos_x && trip->quantum_disponible && trip->continuar) {

		(trip->posicion[0] < pos_x) ? trip->posicion[0]++ : trip->posicion[0]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	while(trip->posicion[1] != pos_y && trip->quantum_disponible && trip->continuar) {

		(trip->posicion[1] < pos_y) ? trip->posicion[1]++ : trip->posicion[1]--;
		avisar_movimiento(trip);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	if(!trip->continuar)
		log_info(logger,"Tripulante %d quitado de running (movimiento)", trip->id_trip);
	else if(trip->posicion[0] == pos_x && trip->posicion[1] == pos_y)
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

	if(trip->continuar) {
		if(MONGO_ACTIVADO) {
			t_mensaje* mensaje_out;

			switch(tarea){
				case GENERAR_OXIGENO:
					mensaje_out = crear_mensaje(GEN_OX);
					break;
				case CONSUMIR_OXIGENO:
					mensaje_out = crear_mensaje(CON_OX);
					break;
				case GENERAR_COMIDA:
					mensaje_out = crear_mensaje(GEN_CO);
					break;
				case CONSUMIR_COMIDA:
					mensaje_out = crear_mensaje(CON_CO);
					break;
				case GENERAR_BASURA:
					mensaje_out = crear_mensaje(GEN_BA);
					break;
				case DESCARTAR_BASURA:
					mensaje_out = crear_mensaje(DES_BA);
					break;
				case ESPERAR: break;
			}

			agregar_parametro_a_mensaje(mensaje_out, (void*)cantidad, ENTERO);
			enviar_y_verificar(mensaje_out, trip->socket_mongo, "Fallo al cargar respuesta en el MONGO");
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

		sem_post(&io_disponible);
		//ciclocpu
		if(!hay_sabotaje)
			agregar_ready(trip);
		else
			agregar_emergencia(trip);

		sem_wait(&trip->sem_running);
	} else
		sem_post(&trip->sem_blocked);
}

bool esperar(int tiempo, tripulante* trip) {
	while(trip->tiempo_esperado < tiempo && trip->quantum_disponible && trip->continuar) {

		trip->tiempo_esperado++;
		log_info(logger,"Tripulante %d: Espero %d de %d",trip->id_trip, trip->tiempo_esperado, tiempo);
		sleep(ciclo_CPU);

		puede_continuar(trip);
	}

	if(!trip->continuar)
		log_info(logger,"Tripulante %d quitado de running (esperar)", trip->id_trip);
	else if(trip->tiempo_esperado == tiempo) {
		trip->tiempo_esperado = 0;
		return true;
	}

	return false;
}
