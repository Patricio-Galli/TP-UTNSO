/*
 ============================================================================
 Name        : discordiador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "discordiador.h"

int variable = 0;

int id_patota_actual = 1;
nodo_tripulante *lista_tripulantes = NULL;
t_config* config;

int main() {
	t_log* logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);

    config = config_create("discordiador.config");
	int socket_ram, socket_mongo = 0;

	socket_ram = crear_conexion_cliente(
		config_get_string_value(config, "IP_MI_RAM_HQ"),
		config_get_string_value(config, "PUERTO_MI_RAM_HQ")
		);
	
	/*socket_mongo = crear_conexion_cliente(
		config_get_string_value(config, "IP_I_MONGO_STORE"),
		config_get_string_value(config, "PUERTO_I_MONGO_STORE")
		);*/
	
	if(!validar_socket(socket_ram, logger) || !validar_socket(socket_mongo, logger)) {
		close(socket_ram);
		close(socket_mongo);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	bool continuar = true;
	char* buffer_consola;
	command_code funcion_consola;
	t_mensaje* mensaje;

	while(continuar) {
		log_info(logger, "entro al while y voy a leer consola");
		buffer_consola = leer_consola();
		funcion_consola = mapStringToEnum(primer_palabra(buffer_consola));
		t_list *respuesta;
		switch(funcion_consola) {
		case INICIAR_PATOTA:
			log_info(logger, "Iniciar patota. Creando mensaje");
			mensaje = crear_mensaje(INIT_P);
			// int valor[3] = {6, 3, 2};
			
			// OJO, hay que mandar un puntero al entero o un puntero al primer elemento si es string
			agregar_parametro_a_mensaje(mensaje, (void *)id_patota_actual, ENTERO);		// id_patota			
			agregar_parametro_a_mensaje(mensaje, (void *)6, ENTERO);		// cant_trip
			agregar_parametro_a_mensaje(mensaje, (void *)2, ENTERO);		// cant_tareas
			agregar_parametro_a_mensaje(mensaje, buffer_consola, BUFFER);	// tarea 1
			agregar_parametro_a_mensaje(mensaje, buffer_consola, BUFFER);	// tarea 2
			
			enviar_mensaje(socket_ram, mensaje);
			
			respuesta = recibir_mensaje(socket_ram);
			if((int)list_get(respuesta, 0) == TODOOK) {
				log_info(logger, "aeeea, sabalero, sabalero");
			}
			if((int)list_get(respuesta, 0) == NO_SPC) {
				log_info(logger, "aeeea, NO SOY sabalero");
			}
			log_info(logger, "Libero mensaje");
			free(mensaje);
			log_info(logger, "Destruyo respuesta");
			list_destroy(respuesta);
			id_patota_actual++;
			break;
		case INICIAR_TRIPULANTE:
			log_info(logger, "Iniciar tripulante. Creando mensaje");
			mensaje = crear_mensaje(INIT_T);

			agregar_parametro_a_mensaje(mensaje, (void *)id_patota_actual - 1, ENTERO);		// id_patota			
			agregar_parametro_a_mensaje(mensaje, (void *)1, ENTERO);					// id_trip
			agregar_parametro_a_mensaje(mensaje, (void *)3, ENTERO);					// posicion_x
			agregar_parametro_a_mensaje(mensaje, (void *)4, ENTERO);					// posicion_y
			
			enviar_mensaje(socket_ram, mensaje);
			
			respuesta = recibir_mensaje(socket_ram);
			if(respuesta == NULL) {
				log_info(logger, "El servidor ha muerto, doy por finalizada esta wea");
				close(socket_ram);
				close(socket_mongo);
				log_destroy(logger);
				return ERROR_CONEXION;
			}
			
			if((int)list_get(respuesta, 0) == SND_PO) {
				int nuevo_puerto = (int)list_get(respuesta, 1);
				
				pthread_t* nuevo_hilo = malloc(sizeof(pthread_t));
				char str_puerto[7];
				sprintf(str_puerto, "%d", (int)nuevo_puerto);
				int socket = crear_conexion_cliente(config_get_string_value(config, "IP_MI_RAM_HQ"), str_puerto);

				pthread_create(nuevo_hilo, NULL, rutina_hilos, (void *)socket);
			}
			if((int)list_get(respuesta, 0) == NO_SPC) {
				log_info(logger, "aeeea, NO SOY sabalero");
			}
			log_info(logger, "Libero mensaje");
			free(mensaje);
			log_info(logger, "Destruyo respuesta");
			list_destroy(respuesta);
			break;
		case EXPULSAR_TRIPULANTE:
			log_info(logger,"Expulsar tripulante ...");
			break;
		case INICIAR_PLANIFICACION:
			log_info(logger,"INICIAR PLANIFICACION");
			break;
		case PAUSAR_PLANIFICACION:
			log_info(logger,"PAUSAR PLANIFICACION");
			break;
		case OBTENER_BITACORA:
			log_info(logger,"OBTENER BITACORA");
			break;
		case EXIT_DISCORDIADOR:
			mensaje = crear_mensaje(64);
			enviar_mensaje(socket_ram, mensaje);
			continuar = false;
			break;
		case ERROR:
			log_error(logger,"COMANDO INVÁLIDO, INTENTE NUEVAMENTE");
		}
		log_info(logger, "Libero buffer");
		free(buffer_consola);
		/*
		int i = 0;
		while(input[i] != NULL){
			free(input[i]);
			i++;
		}
		free(input);*/
	}
	log_destroy(logger);
	return 0;
}

void iniciar_patota(char**input, int* lista_puertos, t_log *logger) {
	int id_trip_actual = 0;
	bool valida = true;
	int *posiciones = malloc(2 * sizeof(int));
	int cantidad_tripulantes = atoi(input[1]);

	log_info(logger,"Iniciando creacion de Patota nro: %d", id_patota_actual);

	for(int iterador = 0; iterador < cantidad_tripulantes; iterador++) { //atoi: ascii to int
		if(valida && input[iterador+3] != NULL) { //iterador+2 nos estaria dando la direccion de inicio del tripulante
			char** auxiliar = string_split(input[iterador+3], "|"); //divide la posicion de x|y a posiciones[0]=x y posiciones[1]=y
			posiciones[0] = atoi(auxiliar[0]);
			posiciones[1] = atoi(auxiliar[1]);
		}
		else {
			posiciones[0] = 0;
			posiciones[1] = 0;
			valida = false;
		}
		tripulante* nuevo_trip = crear_nodo_trip(posiciones);
		nuevo_trip->id_trip = id_trip_actual;
		nuevo_trip->id_patota = id_patota_actual;
		agregar_trip_a_lista(nuevo_trip);
		id_trip_actual++;
		free(nuevo_trip);
	}
	log_info(logger,"Patota nro: %d iniciada. Cantidad de tripulantes: %d",id_patota_actual,id_trip_actual);
	free(posiciones);
	id_patota_actual++;
}

tripulante* crear_nodo_trip(int *posiciones) {
	tripulante* nuevo = malloc(sizeof(tripulante));
	pthread_t nuevo_hilo;
	int *aux = malloc(2 * sizeof(int));
	aux[0] = posiciones[0];
	aux[1] = posiciones[1];
	pthread_create(&nuevo_hilo, NULL, rutina_hilos, aux);
	// Gran memory leak con nuestra variable AUX. RESOLVER!
	nuevo->estado = NEW;
	nuevo->hilo = nuevo_hilo;
	return nuevo;
}

void agregar_trip_a_lista(tripulante* nuevo_trip) {

	nodo_tripulante *nuevo_nodo = malloc(sizeof(nodo_tripulante));
	nuevo_nodo->data = *nuevo_trip;
	nuevo_nodo->sig = NULL;

	if(lista_tripulantes == NULL){
		lista_tripulantes = nuevo_nodo;
	}
	else {
		nodo_tripulante *aux = lista_tripulantes;
		while(aux->sig != NULL){
			aux = aux->sig;
		}
		aux->sig = nuevo_nodo;
	}
}

void* rutina_hilos(void* socket) {
	t_log* logger = log_create("discordiador.log", "HILOX", 1, LOG_LEVEL_DEBUG);
	
	data_socket(socket, logger);
	t_mensaje* mensaje_out = crear_mensaje(TODOOK);
	enviar_mensaje(socket, mensaje_out);
	t_list* mensaje_in = recibir_mensaje(socket);
	if(mensaje_in == NULL) {
		printf("FALLO EN MENSAJE CON HILO RAM\n");
	}
	else
		printf("EL HILO RAM ME RESPONDIO: %d\n", (int)list_get(mensaje_in, 0));
	return 0;
}


void listar_tripulantes(){
	nodo_tripulante* aux = lista_tripulantes;
	printf("----------------------------------------------------------------------------------\n");
	printf("Estado de Tripulantes\n");
	while(aux != NULL){
		printf("Patota: %d\tTripulante: %d\tEstado: %d\n",aux->data.id_patota, aux->data.id_trip,aux->data.estado);
		aux = aux->sig;
	}
	printf("----------------------------------------------------------------------------------\n");
	free(aux);
}