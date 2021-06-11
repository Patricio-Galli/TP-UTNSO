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

int id_patota_actual = 0;
nodo_tripulante *lista_tripulantes = NULL;


int main() {
	t_log* logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);

    t_config* config = config_create("discordiador.config");
	int socket_ram, socket_mongo = 0;

	socket_ram = crear_conexion_cliente(
		config_get_string_value(config, "IP_MI_RAM_HQ"),
		config_get_string_value(config, "PUERTO_MI_RAM_HQ")
		);
	
	/*socket_mongo = crear_conexion_cliente(
		config_get_string_value(config, "IP_I_MONGO_STORE"),
		config_get_string_value(config, "PUERTO_I_MONGO_STORE")
		);*/
	
	if(socket_mongo < 0 || socket_ram < 0) {
		if(socket_ram < 0)
			log_info(logger, "Fallo en la conexión con Mi-RAM-HQ");
		if(socket_mongo < 0)
			log_info(logger, "Fallo en la conexión con I-Mongo-Store");
		close(socket_ram);
		close(socket_mongo);
		return ERROR_CONEXION;
	}

	bool continuar = true;
	char* buffer_consola;
	command_code funcion_consola;

	while(continuar) {
		buffer_consola = leer_consola();
		funcion_consola = mapStringToEnum(primer_palabra(buffer_consola));
		char** input;

		switch(funcion_consola) {
			case INICIAR_PATOTA:
				input = string_split(buffer_consola, " ");
				int* lista_puertos = malloc(sizeof(int) * atoi(input[1]));
				// iniciar_patota()
				// hay que mandar. Op_code [int], cant_trip [int], cant_posicion_init [int], n * strings, (tareas)
					// tareas: cant_tareas [int], strlen(tarea) [int], tarea [str]

				// op_code → iniciar_patota, ubicacion_trip, eliminar_trip, proxima_tarea, etc. → ram... switch(op_code)
				// iniciar_patota , cant_trip , 
				// desde aca hasta el fin de este while
				enviar_mensaje(buffer_consola, socket_ram);
				char* buffer_socket;
				int i = 0;
				// en este while, recibo los puertos y los almaceno en lista_puertos
				while(i<10) {
					recibir_operacion(socket_ram);	// en este momento sirve por compatibilidad
					buffer_socket = recibir_mensaje(socket_ram);
					log_info(logger, "%s, directo del buffer", buffer_socket);
					if(buffer_socket==NULL)
						break;
					lista_puertos[i] = atoi(buffer_socket);
					log_info(logger, "%d, desde lista_puertos", lista_puertos[i]);
					i++;
				}
				
				/*
				if(lista_puertos == NULL) {
					log_info(logger, "no se recibieron puertos :/");
					break;
				}*/
				// n hilos tripulante discordiador 	→ n hilos en el imongo store && n hilos en la ram
				// n hilos tripulante discordiador 	→ 2*n sockets en el discordiador
				//									→ n sockets en el mongo
				//						   			→ n sockets en la ram
				
				iniciar_patota(input, lista_puertos, logger);
				break;
			case LISTAR_TRIPULANTES:
				listar_tripulantes();
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
				continuar = false;
				break;
			case ERROR:
				log_error(logger,"COMANDO INVÁLIDO, INTENTE NUEVAMENTE");
		}
		free(buffer_consola);

		int i = 0;
		while(input[i] != NULL){
			free(input[i]);
			i++;
		}
		free(input);
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

void* rutina_hilos(void* posiciones) {
	/*conectarse_con_ram(mongo);
	conectarse_con_disco(ram);
	// RR definido por el archivo de configuración
	switch(PLANEACION) { // FIFO O RR

	while(tengo_tareas) {
		wait(puedo_trabajar);
		wait(RR);
		pedir_instruccion();
		signal(RR);
		
		informar_bitacora();
		
		wait(RR);
		recibir_instruccion();
		
		signal(puedo_trabajar);
		ejecutar_instruccion();
		signal(puedo_trabajar);

		informar_bitacora();

		if(instruccion == moverse) {
			informar_bitacora();
		}
	}*/
	free(posiciones);
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