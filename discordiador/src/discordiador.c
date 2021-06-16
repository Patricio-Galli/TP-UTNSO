#include "discordiador.h"

int id_patota_actual = 0;
t_list* lista_tripulantes;
t_log* logger;
t_config* config;
bool planificacion_activada;

int main() {
	logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");
	lista_tripulantes = list_create();
	bool continuar = true;

	while(continuar) {
		char* buffer_consola = leer_consola();
		char** input = string_split(buffer_consola, " ");

		command_code funcion = mapStringToEnum(input[0]);

		parametros_iniciar_patota* parametros;

		if (!strcmp(buffer_consola,"ini")) {
			input = string_split("iniciar_patota 4 /home/utnso/tp-2021-1c-cualquier-cosa/tareas.txt 5|3 5|2", " ");
			funcion = mapStringToEnum(input[0]);
		}

		switch(funcion) {
			case INICIAR_PATOTA:
				parametros = obtener_parametros(input); //buffer_consola -> iniciar_patota 4 /home/utnso/tp-2021-1c-cualquier-cosa/tareas.txt 5|3 5|2
				loggear_parametros(parametros);

				iniciar_patota(parametros);
				//liberar_parametros(parametros);
				break;

			case LISTAR_TRIPULANTES:
				listar_tripulantes();
				break;

			case EXPULSAR_TRIPULANTE:
				expulsar_tripulante(atoi(input[1]));
				break;

			case INICIAR_PLANIFICACION:
				iniciar_planificacion();
				break;

			case PAUSAR_PLANIFICACION:
				pausar_planificacion();
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
	//list_destroy_and_destroy_elements(lista_tripulantes, free()); podria ser esta la funcion pero no estoy seguro que funcione
	log_destroy(logger);
	return 0;
}

void iniciar_patota(parametros_iniciar_patota* parametros) {
	log_info(logger,"Iniciando creacion de Patota nro: %d", id_patota_actual);

	for(int iterador = 0; iterador < parametros->cantidad_tripulantes; iterador++) {
		tripulante* nuevo_tripulante = crear_tripulante(parametros->posiciones_x[iterador], parametros->posiciones_y[iterador], id_patota_actual, iterador, logger);

		nuevo_tripulante->posicion_lista = list_add(lista_tripulantes, nuevo_tripulante); //devuelve la posicion en la que se agrego
	}
	log_info(logger,"Patota nro: %d iniciada.",id_patota_actual);

	id_patota_actual++;
}

void listar_tripulantes() {
	int cantidad_elementos = lista_tripulantes->elements_count;

	log_info(logger,"Cantidad de nodos: %d", cantidad_elementos);

	for(int i=0; i < cantidad_elementos; i++) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(lista_tripulantes, i);
		char* estado = estado_enumToString(nuevo_tripulante->estado);

		log_info(logger,"Tripulante: %d    Patota: %d    Status: %s", nuevo_tripulante->id_trip, nuevo_tripulante->id_patota, estado);
	}
}

void expulsar_tripulante(int id_tripulante) {
	log_info(logger,"Expulsando al tripulante %d", id_tripulante);

	bool continuar = true;
	int index = 0;

	while(continuar) {
		tripulante* nuevo_tripulante = (tripulante*)list_get(lista_tripulantes, index);

		log_info(logger,"Tripulante: %d    Patota: %d", nuevo_tripulante->id_trip, nuevo_tripulante->id_patota);

		if(nuevo_tripulante->id_trip == id_tripulante) {
			continuar = false;

			tripulante* tripulante_expulsado = (tripulante*)list_remove(lista_tripulantes, index);

			free(tripulante_expulsado);

			log_info(logger,"El tripulante %d ha sido expulsado", id_tripulante);

			//todo avisar a la ram
		}
		else {
			index++;

			if(index == lista_tripulantes->elements_count) {
				continuar = false;
				log_info(logger,"No existe el tripulante %d", id_tripulante);
			}
		}
	}
}

void iniciar_planificacion() {
	log_info(logger,"Iniciando planificacion...");

	int multiprogramacion = atoi(config_get_string_value(config, "GRADO_MULTITAREA"));
	char* algoritmo = config_get_string_value(config, "ALGORITMO");
	int quantum = atoi(config_get_string_value(config, "QUANTUM"));
	planificacion_activada = true;

	planificador(lista_tripulantes, multiprogramacion, algoritmo, quantum, &planificacion_activada, logger);

	 /*
	if(!strcmp(algoritmo,"FIFO"))
		iniciar_FIFO(multiprogramacion, lista_tripulantes, &planificacion_activada, logger);
	else
		iniciar_RR(multiprogramacion, atoi(config_get_string_value(config, "QUANTUM")), lista_tripulantes, &planificacion_activada, logger);
	*/
}

void pausar_planificacion() {
	log_info(logger,"Pausando planificacion...");
	planificacion_activada = false;
}

parametros_iniciar_patota* obtener_parametros(char** input) {//todo realizar validaciones para lectura de archivos y parametros validos
	log_info(logger,"Obteniendo parametros...");

	parametros_iniciar_patota* parametros = malloc(sizeof(parametros_iniciar_patota));
	bool valida = true;

	int cantidad_tripulantes = atoi(input[1]);

	parametros->cantidad_tripulantes = cantidad_tripulantes;
	parametros->posiciones_x = malloc(cantidad_tripulantes * sizeof(int));
	parametros->posiciones_y = malloc(cantidad_tripulantes * sizeof(int));

	for(int iterador = 0; iterador < cantidad_tripulantes; iterador++) { // completo las posiciones del struct

		if(valida && input[iterador+3] != NULL) { //iterador+3 nos estaria dando la ubicacion de inicio del tripulante
			char** auxiliar = string_split(input[iterador+3], "|"); //divide la posicion de "x|y" a auxiliar[0]=x y auxiliar[1]=y
			parametros->posiciones_x[iterador] = atoi(auxiliar[0]);
			parametros->posiciones_y[iterador] = atoi(auxiliar[1]);
		}
		else {
			parametros->posiciones_x[iterador] = 0;
			parametros->posiciones_y[iterador] = 0;
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

void loggear_parametros(parametros_iniciar_patota* parametros) {
	log_info(logger,"Cantidad de tripulantes: %d", parametros->cantidad_tripulantes);

	for(int i = 0; i < parametros->cantidad_tripulantes; i++)
		log_info(logger,"Tripulante: %d  |  Posicion x: %d  |  Posicion y: %d", i, parametros->posiciones_x[i], parametros->posiciones_y[i]);

	log_info(logger,"Cantidad de tareas: %d", parametros->cantidad_tareas);

	for(int i = 0; i < parametros->cantidad_tareas; i++)
		log_info(logger,"Tarea %d: %s", i, parametros->tareas[i]);

}
