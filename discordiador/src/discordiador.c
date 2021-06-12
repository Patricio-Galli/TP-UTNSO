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

	/*
    t_config* config = config_create("discordiador.config");

    int socket_ram, socket_mongo = 0;

	socket_ram = crear_conexion_cliente(
		config_get_string_value(config, "IP_MI_RAM_HQ"),
		config_get_string_value(config, "PUERTO_MI_RAM_HQ")
		);
	
	socket_mongo = crear_conexion_cliente(
		config_get_string_value(config, "IP_I_MONGO_STORE"),
		config_get_string_value(config, "PUERTO_I_MONGO_STORE")
		);
	
	if(socket_mongo < 0 || socket_ram < 0) {
		if(socket_ram < 0)
			log_info(logger, "Fallo en la conexión con Mi-RAM-HQ");
		if(socket_mongo < 0)
			log_info(logger, "Fallo en la conexión con I-Mongo-Store");
		//close(socket_ram);
		//close(socket_mongo);
		//return ERROR_CONEXION;
	}
	*/

	bool continuar = true;
	char* buffer_consola; //iniciar_patota 4 /home/utnso/tareas_tp_so/tareas.txt 5|3 5|2
	command_code funcion;

	while(continuar) {
		buffer_consola = leer_consola();
		funcion = mapStringToEnum(seleccionar_funcion(buffer_consola));
		parametros_iniciar_patota* parametros;
		switch(funcion) {
			case INICIAR_PATOTA:

				log_info(logger,"Se cargan los parametros en el struct");

				parametros = obtener_parametros(buffer_consola, logger);
				
				log_info(logger,"Cantidad de tripulantes: %d", parametros->cantidad_tripulantes);

				for(int i = 0; i < parametros->cantidad_tripulantes; i++)
					log_info(logger,"Tripulante: %d  |  Posicion x: %d  |  Posicion y: %d", i, parametros->posiciones_tripulantes_x[i], parametros->posiciones_tripulantes_y[i]);

				log_info(logger,"Cantidad de tareas: %d", parametros->cantidad_tareas);

				for(int i = 0; i < parametros->cantidad_tareas; i++)
					log_info(logger,"Tarea %d: %s", i, parametros->tareas[i]);

				log_info(logger,"continuar");
				// iniciar_patota(input, lista_puertos, logger);
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
				log_info(logger,"Exit Discordiador");
				continuar = false;
				break;
			case ERROR:
				log_error(logger,"COMANDO INVÁLIDO, INTENTE NUEVAMENTE");
		}
		log_info(logger,"continuar2");
		free(buffer_consola);
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

parametros_iniciar_patota* obtener_parametros(char* buffer_consola, t_log* logger) {

	parametros_iniciar_patota* parametros = malloc(sizeof(parametros_iniciar_patota));
	bool valida = true;

	char** input = string_split(buffer_consola, " ");

	int cantidad_tripulantes = atoi(input[1]);

	parametros->cantidad_tripulantes = cantidad_tripulantes;
	parametros->posiciones_tripulantes_x = malloc(cantidad_tripulantes * sizeof(int));
	parametros->posiciones_tripulantes_y = malloc(cantidad_tripulantes * sizeof(int));

	//log_info(logger,"Cantidad de tripulantes: %d",parametros->cantidad_tripulantes);

	for(int iterador = 0; iterador < cantidad_tripulantes; iterador++) { // completo las posiciones del struct

		if(valida && input[iterador+3] != NULL) { //iterador+3 nos estaria dando la ubicacion de inicio del tripulante
			char** auxiliar = string_split(input[iterador+3], "|"); //divide la posicion de "x|y" a auxiliar[0]=x y auxiliar[1]=y
			parametros->posiciones_tripulantes_x[iterador] = atoi(auxiliar[0]);
			parametros->posiciones_tripulantes_y[iterador] = atoi(auxiliar[1]);
		}
		else {
			parametros->posiciones_tripulantes_x[iterador] = 0;
			parametros->posiciones_tripulantes_y[iterador] = 0;
			valida = false;
		}
		//log_info(logger,"Tripulante: %d  posicion x: %d  posicion y: %d",iterador, parametros->posiciones_tripulantes_x[iterador], parametros->posiciones_tripulantes_y[iterador]);
	}

	//log_info(logger,"Path del archivo tareas: %s",input[2]);

	FILE *archivo_tareas = fopen (input[2], "r");
	char buffer_tarea[40];
	parametros->cantidad_tareas = 0;
	parametros->tareas = NULL;

	while (fgets(buffer_tarea, 100, archivo_tareas)) {

		strtok(buffer_tarea, "\n"); //strtok le saca el \n al string de buffer_tarea que es agregado por fgets al leer del archivo
		//log_info(logger,"Buffer tarea %d: %s",parametros->cantidad_tareas, buffer_tarea);

		parametros->tareas = realloc(parametros->tareas, (parametros->cantidad_tareas + 1) * sizeof(char*)); //le agrego a mi vector de tareas[str] un nuevo valor para colocar una nueva tarea

		parametros->tareas[parametros->cantidad_tareas] = malloc(sizeof(buffer_tarea));

		memcpy(parametros->tareas[parametros->cantidad_tareas], &buffer_tarea, sizeof(buffer_tarea));

		//log_info(logger,"nueva tarea numero %d cargada en el struct: %s",parametros->cantidad_tareas,parametros->tareas[parametros->cantidad_tareas]);

		parametros->cantidad_tareas++;
	}

	fclose(archivo_tareas);

	return parametros;
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
