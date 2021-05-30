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

sem_t sem_posiciones;
int variable = 0;

int patota_actual = 0;

int main() {

    char** input;
    bool continuar = true;

	while(continuar) {
		input = leer_consola();
		command_code command = mapStringToEnum(input[0]);

		switch(command) {
			case INICIAR_PATOTA:
				iniciar_patota(input);
				break;
			case LISTAR_TRIPULANTES:
				printf("LISTAR TRIPULANTES RUNNING");
				break;
			case EXPULSAR_TRIPULANTE:
				printf("EXPULSAR");
				break;
			case INICIAR_PLANIFICACION:
				printf("INICIAR PLANIFICACION");
				break;
			case PAUSAR_PLANIFICACION:
				printf("PAUSAR PLANIFICACION");
				break;
			case OBTENER_BITACORA:
				printf("OBTENER BITACORA");
				break;
			case EXIT_DISCORDIADOR:
				continuar = false;
				break;
			case ERROR:
				printf("COMANDO INVÁLIDO, INTENTE NUEVAMENTE");
		}
		
		/*tripulante* iterador = lista_trip->cabeza;
		if (iterador != NULL) {
			while(iterador->sig != NULL) {
				pthread_join(iterador->hilo, NULL);
				printf("y\n");
				iterador = iterador->sig;
			}
		}*/
		free(*input);
		free(input);
	}
	return 0;
}

void iniciar_patota(char** input){

	bool valida = true;
	int posiciones[2];
	int cantidad_tripulantes = atoi(input[1]);
	sem_init(&sem_posiciones, 0, 1);
	lista_tripulante* lista_trip = malloc(sizeof(lista_tripulante));

	printf("INICIAR PATOTA RUNNING\n");

	for(int iterador = 0; iterador < cantidad_tripulantes; iterador++) { //atoi: ascii to int
		sem_wait(&sem_posiciones);
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
		printf("(%d, %d)\n", posiciones[0], posiciones[1]);
		tripulante* nuevo_trip = crear_nodo_trip(posiciones);
		agregar_trip_a_lista(nuevo_trip, lista_trip, patota_actual);
	}
}

command_code mapStringToEnum(char *string){
	// hago que valide indistintamente las mayúsculas y minpusculas
	//string_to_upper(string);

	char* listaDeStrings[]={"INICIAR_PATOTA", "LISTAR_TRIPULANTES", "EXPULSAR_TRIPULANTE", "INICIAR_PLANIFICACION", "PAUSAR_PLANIFICACION", "OBTENER_BITACORA", "EXIT"};

	for(int i=0;i<7;i++){
		if(!strcasecmp(string,listaDeStrings[i]))
			return i;
	}
	return ERROR;
}

tripulante* crear_nodo_trip(int posiciones[2]) {
	printf("Creando tripulante pro\n");
	tripulante* nuevo = malloc(sizeof(tripulante));
	pthread_t nuevo_hilo;
	pthread_create(&nuevo_hilo, NULL, rutina_hilos, posiciones);

	nuevo->estado = NEW;
	nuevo->hilo = nuevo_hilo;
	nuevo->sig = NULL;
	return nuevo;
}

void agregar_trip_a_lista(tripulante* nuevo_trip, lista_tripulante* lista, int patota) {
	int id = 0;
	if(lista->cabeza == NULL) {
		lista->cabeza = nuevo_trip;
	}
	else {
		tripulante* nodo = lista->cabeza;
		while(nodo->sig) {
			nodo = nodo->sig;
		}
		nodo->sig = nuevo_trip;
	}
	nuevo_trip->id_trip = id;
	nuevo_trip->id_patota = patota;
}

void* rutina_hilos(int* posiciones) {
	printf("Hola soy el hilo %d, mi posición es (%d, %d)\n", variable, posiciones[0], posiciones[1]);
	variable++;
	sem_post(&sem_posiciones);
	return 0;
}
