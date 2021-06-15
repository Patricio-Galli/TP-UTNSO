#include "tripulante.h"

tripulante* crear_tripulante(int x, int y, int patota, int id,t_log* logger) {
	tripulante* nuevo_tripulante = malloc(sizeof(tripulante));
	pthread_t nuevo_hilo;

	pthread_create(&nuevo_hilo, NULL, rutina_hilos, nuevo_tripulante);

	nuevo_tripulante->id_trip = id;
	nuevo_tripulante->id_patota = patota;
	nuevo_tripulante->estado = NEW;
	nuevo_tripulante->hilo = nuevo_hilo;
	nuevo_tripulante->posicion[0] = x;
	nuevo_tripulante->posicion[1] = y;

	return nuevo_tripulante;
}

void* rutina_hilos(void* tripulante) {
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
	return 0;
}

char* enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}
