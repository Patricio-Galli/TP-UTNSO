#include "discordiador.h"

int main() {
	logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	ip_ram = config_get_string_value(config, "IP_MI_RAM_HQ");
	ip_mongo = config_get_string_value(config, "IP_I_MONGO_STORE");

	if(RAM_ACTIVADA) {
		socket_ram = crear_conexion_cliente(ip_ram,	config_get_string_value(config, "PUERTO_MI_RAM_HQ"));

		if(!validar_socket(socket_ram, logger)) {
			close(socket_ram);
			log_destroy(logger);
			return 0;
		}
	}

	if(MONGO_ACTIVADO) {
		socket_mongo = crear_conexion_cliente(ip_mongo, config_get_string_value(config, "PUERTO_I_MONGO_STORE"));

		if(!validar_socket(socket_mongo, logger)) {
			close(socket_mongo);
			log_destroy(logger);

			if(RAM_ACTIVADA)
				close(socket_ram);
			return 0;
		}

		log_info(logger,"Iniciando activacion detector sabotajes");

		t_mensaje* mensaje_activacion_out = crear_mensaje(INIT_S);
		enviar_mensaje(socket_mongo, mensaje_activacion_out);
		t_list* mensaje_activacion_in = recibir_mensaje(socket_mongo);

		if((int)list_get(mensaje_activacion_in, 0) == SND_PO) {
			char puerto[7];
			sprintf(puerto, "%d", (int)list_get(mensaje_activacion_in, 1));

			int socket_sabotaje = crear_conexion_cliente(ip_mongo, puerto);

			pthread_t hilo_detector_sabotaje;
			pthread_create(&hilo_detector_sabotaje, NULL, detector_sabotaje, &socket_sabotaje);
		}else
			log_error(logger, "No se pudo activar el detector de  sabotajes, fallo en el mongo.");

		liberar_mensaje_out(mensaje_activacion_out);
		liberar_mensaje_in(mensaje_activacion_in);
	}

	lista_tripulantes = list_create();
	salir = false;
	planificacion_inicializada = false;
	continuar_planificacion = true;
	ciclo_CPU = atoi(config_get_string_value(config, "RETARDO_CICLO_CPU"));
	quantum = atoi(config_get_string_value(config, "QUANTUM"));

	inicializar_planificador(
		atoi(config_get_string_value(config, "GRADO_MULTITAREA")),
		config_get_string_value(config, "ALGORITMO"));

	while(!salir) {
		char* buffer_consola = leer_consola();
		char** input = string_split(buffer_consola, " ");

		switch(mapStringToEnum(input[0])) {
			case INICIAR_PATOTA:

				if (!strcmp(buffer_consola,"ini")) {
					free(input);
					input = string_split("iniciar_patota 4 /home/utnso/tp-2021-1c-cualquier-cosa/tareas.txt 9|3 9|2", " ");
				}

				parametros_iniciar_patota* parametros = obtener_parametros(input);
				loggear_parametros(parametros);

				if(RAM_ACTIVADA || MONGO_ACTIVADO) {
					bool ram_ok = true, mongo_ok = true;

					t_mensaje* mensaje_out = crear_mensaje(INIT_P);

					agregar_parametro_a_mensaje(mensaje_out, (void*)parametros->cantidad_tripulantes, ENTERO);
					agregar_parametro_a_mensaje(mensaje_out, (void*)parametros->cantidad_tareas, ENTERO);
					for(int i = 0; i < parametros->cantidad_tareas; i++)
						agregar_parametro_a_mensaje(mensaje_out, (void*)parametros->tareas[i], BUFFER);

					if(RAM_ACTIVADA) {
						enviar_mensaje(socket_ram, mensaje_out);
						t_list* mensaje_in = recibir_mensaje(socket_ram);

						ram_ok = respuesta_OK(mensaje_in, "No se pudo iniciar patota, no hay suficiente memoria.");
						liberar_mensaje_in(mensaje_in);
					}

					if(MONGO_ACTIVADO) {
						enviar_mensaje(socket_mongo, mensaje_out);
						t_list* mensaje_in = recibir_mensaje(socket_mongo);

						mongo_ok = respuesta_OK(mensaje_in, "No se pudo iniciar patota, error en mongo.");
						liberar_mensaje_in(mensaje_in);
					}

					if(ram_ok && mongo_ok)
						iniciar_patota(parametros);

					liberar_mensaje_out(mensaje_out);

				} else
					iniciar_patota(parametros);

				liberar_parametros(parametros);
				break;

			case LISTAR_TRIPULANTES:
				listar_tripulantes();
				break;

			case EXPULSAR_TRIPULANTE:
				expulsar_tripulante(atoi(input[1]),atoi(input[2]));
				break;

			case INICIAR_PLANIFICACION:
				iniciar_planificacion();
				break;

			case PAUSAR_PLANIFICACION:
				pausar_planificacion();
				break;

			case OBTENER_BITACORA:
				obtener_bitacora(atoi(input[1]), atoi(input[2]));
				break;

			case EXIT_DISCORDIADOR:
				log_info(logger,"Exit Discordiador");
				salir = true;
				break;

			case ERROR:
				log_error(logger,"COMANDO INVÁLIDO, INTENTE NUEVAMENTE");
		}
		free(buffer_consola);
		liberar_input(input);

	}

	for(int i = 0; i < list_size(lista_tripulantes); i++) {
		tripulante* trip = (tripulante*)list_get(lista_tripulantes, i);
		trip->continuar = false;
		sem_post(&trip->sem_running);
		sem_post(&trip->sem_blocked);

		//pthread_join(trip->hilo);
	}

	exit_planificacion();
	//liberar_tripulantes();
	//exit_tripulantes();

	close(socket_ram);
	close(socket_mongo);
	log_destroy(logger);
	return 0;
}

void iniciar_patota(parametros_iniciar_patota* parametros) {
	log_info(logger,"Iniciando creacion de Patota nro: %d", id_patota_actual);

	for(int iterador = 1; iterador <= parametros->cantidad_tripulantes; iterador++) {
		if(RAM_ACTIVADA || MONGO_ACTIVADO) {
			int socket_ram_trip = 0, socket_mongo_trip = 0;

			t_mensaje* mensaje_out = crear_mensaje(INIT_T);

			agregar_parametro_a_mensaje(mensaje_out, (void*)parametros->posiciones_x[iterador-1], ENTERO);
			agregar_parametro_a_mensaje(mensaje_out, (void*)parametros->posiciones_y[iterador-1], ENTERO);

			if(RAM_ACTIVADA) {
				enviar_mensaje(socket_ram, mensaje_out);
				t_list* mensaje_in = recibir_mensaje(socket_ram);

				if((int)list_get(mensaje_in, 0) == SND_PO) {
					char puerto[7];
					sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));

					socket_ram_trip = crear_conexion_cliente(ip_ram, puerto);
				} else
					log_error(logger, "No se pudo crear al tripulante, no hay suficiente memoria.");

				liberar_mensaje_in(mensaje_in);
			}

			if(MONGO_ACTIVADO) {
				enviar_mensaje(socket_mongo, mensaje_out);
				t_list* mensaje_in = recibir_mensaje(socket_mongo);

				if((int)list_get(mensaje_in, 0) == SND_PO) {
					char puerto[7];
					sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));

					socket_mongo_trip = crear_conexion_cliente(ip_mongo, puerto);
				} else
					log_error(logger, "No se pudo crear al tripulante, fallo en el disco.");

				liberar_mensaje_in(mensaje_in);
			}

			liberar_mensaje_out(mensaje_out);

			tripulante* nuevo_tripulante = crear_tripulante(
					parametros->posiciones_x[iterador-1],
					parametros->posiciones_y[iterador-1],
					id_patota_actual,
					iterador,
					socket_ram_trip,
					socket_mongo_trip);


			list_add(lista_tripulantes, nuevo_tripulante); //devuelve la posicion en la que se agrego

		} else {
			tripulante* nuevo_tripulante = crear_tripulante(parametros->posiciones_x[iterador-1], parametros->posiciones_y[iterador-1], id_patota_actual, iterador, 0, 0);
			list_add(lista_tripulantes, nuevo_tripulante);
		}
	}
	log_info(logger,"Patota nro: %d iniciada.\n",id_patota_actual);

	id_patota_actual++;
}

void listar_tripulantes() {
	log_info(logger,"Cantidad de tripulantes: %d", lista_tripulantes->elements_count);

	for(int i=0; i < lista_tripulantes->elements_count; i++) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(lista_tripulantes, i);
		char* estado = estado_enumToString(nuevo_tripulante->estado);

		log_info(logger,"\t Tripulante: %d    Patota: %d    Posicion: %d|%d    Status: %s", nuevo_tripulante->id_trip, nuevo_tripulante->id_patota, nuevo_tripulante->posicion[0], nuevo_tripulante->posicion[1], estado);
	}
}

void expulsar_tripulante(int id_tripulante, int id_patota) {
	log_info(logger,"Expulsando al tripulante %d de la patota %d", id_tripulante, id_patota);

	bool continuar = true;
	int index = 0;

	while(continuar) {
		tripulante* trip = (tripulante*)list_get(lista_tripulantes, index);

		if(trip->id_trip == id_tripulante && trip->id_patota == id_patota) {
			bool eliminar_trip = true;
			continuar = false;

			if(RAM_ACTIVADA) {
				t_mensaje* mensaje_out = crear_mensaje(ELIM_T);

				agregar_parametro_a_mensaje(mensaje_out, (void*)id_tripulante, ENTERO);
				agregar_parametro_a_mensaje(mensaje_out, (void*)id_patota, ENTERO);

				eliminar_trip = enviar_y_verificar(mensaje_out, socket_ram, "No se pudo expulsar al tripulante.");
			}

			if(eliminar_trip) {
				trip->continuar = false;

				switch(trip->estado) {
					case READY:
						sem_post(&trip->sem_running);

						if(!continuar_planificacion)
							sem_post(&trip->sem_running);

						//pthread_join(trip->hilo, NULL);
						sem_wait(&trip->sem_blocked);
						quitar(trip, cola_ready);
						sem_wait(&tripulantes_ready);
						break;
					case BLOCKED:
						sem_post(&trip->sem_blocked);

						if(!continuar_planificacion)
							sem_post(&trip->sem_running);

						//pthread_join(trip->hilo);
						sem_wait(&trip->sem_blocked);
						quitar(trip, cola_blocked);
						sem_wait(&tripulantes_ready);
						break;
					case RUNNING:
						if(!continuar_planificacion)
							sem_post(&trip->sem_running);

						//pthread_join(trip->hilo);
						sem_wait(&trip->sem_blocked);
						quitar_running(trip);
						break;
					default: break;
				}

				free(list_remove(lista_tripulantes, index));
				log_info(logger,"El tripulante %d de la patota %d ha sido expulsado", id_tripulante, id_patota);
			}
		}
		else {
			index++;

			if(index == lista_tripulantes->elements_count) {
				continuar = false;
				log_error(logger,"No existe el tripulante %d de la patota %d", id_tripulante, id_patota);
			}
		}
	}
}

void iniciar_planificacion() {
	if(!planificacion_inicializada) {
		log_info(logger,"Iniciando planificacion...");
		planificacion_inicializada = true;
		sem_post(&activar_planificacion);
	}
	else{
		log_info(logger,"Reiniciando planificacion...");
		continuar_planificacion = true;
		for(int i = 0; i < list_size(tripulantes_running); i++) {
			tripulante* trip = (tripulante*)list_get(tripulantes_running, i);
			sem_post(&trip->sem_running);
		}

		if(trip_block != NULL)
			sem_post(&trip_block->sem_running);
	}
}

void pausar_planificacion() {
	log_info(logger,"Pausando planificacion...");
	continuar_planificacion = false;
}

void obtener_bitacora(int tripulante, int patota) {
	log_info(logger,"Obteniendo bitacora del tripulante %d de la patota %d", tripulante, patota);

	if(MONGO_ACTIVADO) {
		t_mensaje* mensaje_out = crear_mensaje(BITA_D);
		agregar_parametro_a_mensaje(mensaje_out, (void*)tripulante, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)patota, ENTERO);

		enviar_mensaje(socket_mongo, mensaje_out);
		t_list* mensaje_in = recibir_mensaje(socket_mongo);

		if((int)list_get(mensaje_in, 0) == BITA_C) {
			for(int i = 0; i<(int)list_get(mensaje_in, 1); i++) {
				log_info(logger,"%s", (char*)list_get(mensaje_in, i+2));
			}
		} else
			log_error(logger, "No se pudo obtener la bitacora solicitada");
	}
}
