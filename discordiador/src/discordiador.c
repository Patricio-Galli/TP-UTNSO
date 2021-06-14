#include "discordiador.h"

int id_patota_actual = 0;

int main() {
	t_log* logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);


	bool continuar = true;
	char* buffer_consola;
	command_code funcion;

	while(continuar) {
		buffer_consola = leer_consola();
		funcion = mapStringToEnum(seleccionar_funcion(buffer_consola));
		parametros_iniciar_patota* parametros;
		switch(funcion) {
			case INICIAR_PATOTA:

				log_info(logger,"Se cargan los parametros en el struct");

				parametros = obtener_parametros(buffer_consola); //buffer_consola -> iniciar_patota 4 /home/utnso/tp-2021-1c-cualquier-cosa/tareas.txt 5|3 5|2

				log_info(logger,"Cantidad de tripulantes: %d", parametros->cantidad_tripulantes);

				for(int i = 0; i < parametros->cantidad_tripulantes; i++)
					log_info(logger,"Tripulante: %d  |  Posicion x: %d  |  Posicion y: %d", i, parametros->posiciones_tripulantes_x[i], parametros->posiciones_tripulantes_y[i]);

				log_info(logger,"Cantidad de tareas: %d", parametros->cantidad_tareas);

				for(int i = 0; i < parametros->cantidad_tareas; i++)
					log_info(logger,"Tarea %d: %s", i, parametros->tareas[i]);

				// iniciar_patota(input, lista_puertos, logger);
				break;
			case LISTAR_TRIPULANTES:
				//listar_tripulantes();
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
	nodo_tripulante *lista_tripulantes = NULL;

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
		agregar_trip_a_lista(nuevo_trip, lista_tripulantes);
		id_trip_actual++;
		free(nuevo_trip);
	}
	log_info(logger,"Patota nro: %d iniciada. Cantidad de tripulantes: %d",id_patota_actual,id_trip_actual);
	free(posiciones);
	id_patota_actual++;
}



parametros_iniciar_patota* obtener_parametros(char* buffer_consola) {//todo realizar validaciones para lectura de archivos y parametros validos

	parametros_iniciar_patota* parametros = malloc(sizeof(parametros_iniciar_patota));
	bool valida = true;

	char** input = string_split(buffer_consola, " ");

	int cantidad_tripulantes = atoi(input[1]);

	parametros->cantidad_tripulantes = cantidad_tripulantes;
	parametros->posiciones_tripulantes_x = malloc(cantidad_tripulantes * sizeof(int));
	parametros->posiciones_tripulantes_y = malloc(cantidad_tripulantes * sizeof(int));

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
	}

	FILE *archivo_tareas = fopen (input[2], "r");
	char buffer_tarea[40];
	parametros->cantidad_tareas = 0;
	parametros->tareas = NULL;

	while (fgets(buffer_tarea, 100, archivo_tareas)) {
		strtok(buffer_tarea, "\n"); //strtok le saca el \n al string de buffer_tarea que es agregado por fgets al leer del archivo

		parametros->tareas = realloc(parametros->tareas, (parametros->cantidad_tareas + 1) * sizeof(char*)); //le agrego a mi vector de tareas[str] un nuevo valor para colocar una nueva tarea
		parametros->tareas[parametros->cantidad_tareas] = malloc(sizeof(buffer_tarea));

		memcpy(parametros->tareas[parametros->cantidad_tareas], &buffer_tarea, sizeof(buffer_tarea));

		parametros->cantidad_tareas++;
	}

	fclose(archivo_tareas);

	return parametros;
}
