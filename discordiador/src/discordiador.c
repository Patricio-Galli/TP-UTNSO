/*
	============================================================================
	Name		:	discordiador.c
	Author		: 
	Version		:
	Copyright	:	Your copyright notice
	Description	:	Hello World in C, Ansi-style
	============================================================================
*/

#include "discordiador.h"

sem_t semaforo_tripulante;

int id_patota_actual = 1;
int id_tripulante = 1;
nodo_tripulante *lista_tripulantes = NULL;
t_config* config;
int variable = 1;

int main() {
	t_log* logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);

    config = config_create("discordiador.config");
	int socket_ram, socket_mongo = 0;

	socket_ram = crear_conexion_cliente(
		config_get_string_value(config, "IP_MI_RAM_HQ"),
		config_get_string_value(config, "PUERTO_MI_RAM_HQ")
		);
	
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

	char tarea1[] = "Soy la tarea 1\n";
	char tarea2[] = "Soy la tarea 2\n";
	char tarea3[] = "Soy la tarea 3\n";
	int variable = 0;
	while(continuar) {
		buffer_consola = leer_consola();
		variable++;
		/*
		funcion_consola = mapStringToEnum(primer_palabra(buffer_consola));*/
		switch (variable) {
		case 1:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 2:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 3:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 4:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 5:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 6:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 7:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 8:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 9:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 10:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 11:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;
		case 12:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;
		case 13:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;
		case 14:
			funcion_consola = EXPULSAR_TRIPULANTE;
			break;	
		case 15:
			// funcion_consola = INICIAR_PATOTA;
			funcion_consola = ERROR;
			break;
		case 16:
			// funcion_consola = INICIAR_TRIPULANTE;
			funcion_consola = ERROR;
			break;
		case 17:
			// funcion_consola = INICIAR_TRIPULANTE;
			funcion_consola = ERROR;
			break;
		case 18:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 19:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 20:
			funcion_consola = INICIAR_PATOTA;
			break;
		case 21:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		case 22:
			funcion_consola = INICIAR_TRIPULANTE;
			break;
		default:
			log_info(logger, "No hay más instrucciones");
			funcion_consola = EXIT_DISCORDIADOR;
			break;
		}
		t_list *respuesta;
		switch(funcion_consola) {
		case INICIAR_PATOTA:
			log_info(logger, "Iniciar patota. Creando mensaje");
			mensaje = crear_mensaje(INIT_P);
			agregar_parametro_a_mensaje(mensaje, (void *)2, ENTERO);		// cant_tareas
			agregar_parametro_a_mensaje(mensaje, &tarea1, BUFFER);	// tarea 1
			agregar_parametro_a_mensaje(mensaje, &tarea2, BUFFER);	// tarea 2
			
			enviar_mensaje(socket_ram, mensaje);
			
			respuesta = recibir_mensaje(socket_ram);
			if((int)list_get(respuesta, 0) == TODOOK) {
				log_info(logger, "aeeea, sabalero, sabalero");
			}
			if((int)list_get(respuesta, 0) == NO_SPC) {
				log_info(logger, "El servidor dice que no había espacio");
			}
			free(mensaje);
			list_destroy(respuesta);
			id_patota_actual++;
			id_tripulante = 1;
			break;
		case INICIAR_TRIPULANTE:
			log_info(logger, "Iniciar tripulante. Creando mensaje");
			mensaje = crear_mensaje(INIT_T);
			agregar_parametro_a_mensaje(mensaje, (void *)3, ENTERO);						// posicion_x
			agregar_parametro_a_mensaje(mensaje, (void *)4, ENTERO);						// posicion_y
			
			enviar_mensaje(socket_ram, mensaje);
			
			respuesta = recibir_mensaje(socket_ram);
			if(!validar_mensaje(respuesta, logger)) {
				log_info(logger, "El servidor ha muerto, doy por finalizada esta wea");
				close(socket_ram);
				close(socket_mongo);
				log_destroy(logger);
				return ERROR_CONEXION;
			}
			
			id_tripulante++;
			if((int)list_get(respuesta, 0) == SND_PO) {
				int nuevo_puerto = (int)list_get(respuesta, 1);
				
				pthread_t* nuevo_hilo = malloc(sizeof(pthread_t));
				char str_puerto[7];
				sprintf(str_puerto, "%d", (int)nuevo_puerto);
				int socket = crear_conexion_cliente(config_get_string_value(config, "IP_MI_RAM_HQ"), str_puerto);
				sem_init(&semaforo_tripulante, 0, 0);
				pthread_create(nuevo_hilo, NULL, rutina_hilos, (void *)socket);
				// close(socket);
			}
			if((int)list_get(respuesta, 0) == NO_SPC) {
				log_info(logger, "El servidor dice que no había espacio");
			}
			free(mensaje);
			list_destroy(respuesta);
			break;
		case EXPULSAR_TRIPULANTE:
			log_info(logger,"Expulsar tripulante");
			mensaje = crear_mensaje(ELIM_T);
			log_info(logger, "Cree mensaje %d", funcion_consola);
			if(variable == 11) {
				log_info(logger,"Expulsar tripulante 1");
				agregar_parametro_a_mensaje(mensaje, (void *)1, ENTERO);
				agregar_parametro_a_mensaje(mensaje, (void *)1, ENTERO);
			}
			if(variable == 12) {
				log_info(logger,"Expulsar tripulante 2");
				agregar_parametro_a_mensaje(mensaje, (void *)1, ENTERO);
				agregar_parametro_a_mensaje(mensaje, (void *)3, ENTERO);
			}
			if(variable == 13) {
				log_info(logger,"Expulsar tripulante 3");
				agregar_parametro_a_mensaje(mensaje, (void *)1, ENTERO);
				agregar_parametro_a_mensaje(mensaje, (void *)2, ENTERO);
			}
			if(variable == 14) {
				log_info(logger,"Expulsar tripulante 4");
				agregar_parametro_a_mensaje(mensaje, (void *)3, ENTERO);
				agregar_parametro_a_mensaje(mensaje, (void *)2, ENTERO);
			}
			/*if(variable == 18) {
				log_info(logger,"Expulsar tripulante 5");
				agregar_parametro_a_mensaje(mensaje, (void *)2, ENTERO);
				agregar_parametro_a_mensaje(mensaje, (void *)3, ENTERO);
			}*/
			enviar_mensaje(socket_ram, mensaje);
			free(mensaje);
			respuesta = recibir_mensaje(socket_ram);
			if(!validar_mensaje(respuesta, logger)) {
				log_info(logger, "El servidor ha muerto, doy por finalizada esta wea");
				close(socket_ram);
				close(socket_mongo);
				log_destroy(logger);
				return ERROR_CONEXION;
			}
			list_destroy(respuesta);
			break;
		case INICIAR_PLANIFICACION:
			sem_post(&semaforo_tripulante);
			break;
		case PAUSAR_PLANIFICACION:
			
			break;
		case OBTENER_BITACORA:
			log_info(logger,"OBTENER BITACORA");
			break;
		case EXIT_DISCORDIADOR:
			mensaje = crear_mensaje(64);
			enviar_mensaje(socket_ram, mensaje);
			sem_destroy(&semaforo_tripulante);
			continuar = false;
			break;
		case ERROR:
			log_error(logger,"COMANDO INVALIDO, INTENTE NUEVAMENTE");
		}
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
	log_info(logger, "HOLA MUNDO, SOY UN HILO %d", variable);
	variable++;
	
	while(1) {
		sem_wait(&semaforo_tripulante);
		t_mensaje* mensaje_out = crear_mensaje(NEXT_T);
		enviar_mensaje((int)socket, mensaje_out);
		t_list* mensaje_in = recibir_mensaje((int)socket);
		if(!validar_mensaje(mensaje_in, logger))
			log_warning(logger, "FALLO EN MENSAJE CON HILO RAM\n");
		else {
			log_info(logger, "Me llegó %d", (int)list_get(mensaje_in, 0));
			if((int)list_get(mensaje_in, 0) == ER_MSJ)
				log_info(logger, "No hay próxima tarea");
			if((int)list_get(mensaje_in, 0) == TASK_T)
				log_info(logger, "EL HILO RAM ME RESPONDIO: %s\n", (int)list_get(mensaje_in, 1));
		}
	}
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

/*
patota 1 -> dos hilos
patota 2 o cualquiera -> cuatro hilos
dos hilos siempre

*/