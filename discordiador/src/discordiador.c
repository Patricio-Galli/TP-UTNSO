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
				listar_tripulantes();
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

		int i = 0;
		while(input[i] != NULL){
			free(input[i]);
			i++;
		}
		free(input);
	}
	return 0;
}

void iniciar_patota(char** input){

	int id_trip_actual = 0;
	bool valida = true;
	int *posiciones = malloc(2 * sizeof(int));
	int cantidad_tripulantes = atoi(input[1]);

	printf("INICIAR PATOTA RUNNING\n");

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
		printf("(%d, %d)\n", posiciones[0], posiciones[1]);
		tripulante* nuevo_trip = crear_nodo_trip(posiciones);
		nuevo_trip->id_trip = id_trip_actual;
		nuevo_trip->id_patota = id_patota_actual;
		agregar_trip_a_lista(nuevo_trip);
		id_trip_actual++;
		free(nuevo_trip);
	}
	free(posiciones);
	id_patota_actual++;
}

command_code mapStringToEnum(char *string){
	char* listaDeStrings[]={"INICIAR_PATOTA", "LISTAR_TRIPULANTES", "EXPULSAR_TRIPULANTE", "INICIAR_PLANIFICACION", "PAUSAR_PLANIFICACION", "OBTENER_BITACORA", "EXIT"};

	for(int i=0;i<7;i++){
		if(!strcasecmp(string,listaDeStrings[i]))
			return i;
	}
	return ERROR;
}

tripulante* crear_nodo_trip(int *posiciones) {
	printf("Creando tripulante pro\n");
	tripulante* nuevo = malloc(sizeof(tripulante));
	pthread_t nuevo_hilo;
	int *aux = malloc(2 * sizeof(int));
	aux[0] = posiciones[0];
	aux[1] = posiciones[1];
	pthread_create(&nuevo_hilo, NULL, rutina_hilos, aux);

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

void* rutina_hilos(int* posiciones) {
	printf("Hola soy el hilo %d, mi posición es (%d, %d)\n", variable, posiciones[0], posiciones[1]);
	free(posiciones);
	return 0;
}

void listar_tripulantes(){
	nodo_tripulante* aux = lista_tripulantes;
	while(aux != NULL){
		printf("Patota: %d \t Tripulante: %d \n",aux->data.id_patota, aux->data.id_trip);
		aux = aux->sig;
	}
	free(aux);
}
