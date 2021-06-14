#include "hilos.h"

tripulante* crear_tripulante(int x, int y, int patota, int id) {
	tripulante* nuevo = malloc(sizeof(tripulante));
	pthread_t nuevo_hilo;
	int *aux = malloc(2 * sizeof(int));
	aux[0] = x;
	aux[1] = y;
	pthread_create(&nuevo_hilo, NULL, rutina_hilos, aux);
	nuevo->estado = NEW;
	nuevo->hilo = nuevo_hilo;
	nuevo->id_patota = patota;
	nuevo->id_trip = id;

	return nuevo;
}

void agregar_trip_a_lista(tripulante* nuevo_trip, nodo_tripulante *lista_tripulantes) {

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
