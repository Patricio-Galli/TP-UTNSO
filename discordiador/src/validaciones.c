#include "validaciones.h"

/////////////////////VALIDACIONES//////////////////////
void actualizar_quantum(tripulante* trip) {
	trip->contador_ciclos++;

	if(analizar_quantum && trip->contador_ciclos == quantum)
		trip->quantum_disponible = false;
}

void puede_continuar(tripulante* trip) {
	actualizar_quantum(trip);

	if(!continuar_planificacion) {
		log_info(logger,"Tripulante %d pausado", trip->id_trip);
		sem_wait(&trip->sem_running);
		log_info(logger,"Tripulante %d reactivado", trip->id_trip);
	}
}


/////////////////////UTILIDADES//////////////////////
char* estado_enumToString(int estadoEnum) {
	char* listaDeStrings[] = {"NEW", "BLOCKED", "READY", "RUNNING", "EXIT"};

	return listaDeStrings[estadoEnum];
}

tareas stringToEnum(char *string){
	char* listaDeStrings[]={"GENERAR_OXIGENO", "CONSUMIR_OXIGENO", "GENERAR_COMIDA", "CONSUMIR_COMIDA", "GENERAR_BASURA", "DESCARTAR_BASURA"};

	for(int i=0;i<6;i++){
		if(!strcasecmp(string,listaDeStrings[i])) {
			return i;
		}
	}
	return ESPERAR;
}

void liberar_input(char** input) {
	int i = 0;

	while(input[i] != NULL) {
		free(input[i]);
		i++;
	}

	log_info(logger, "%d", i);

	free(input);
}

int distancia_a(tripulante* trip, int pos_x, int pos_y) {
	//return (int) sqrt(pow(pos_x-trip->posicion[0], 2) + pow(pos_y-trip->posicion[1], 2));
	int distancia, resta_x = pos_x-trip->posicion[0], resta_y = pos_y-trip->posicion[1];

	if(resta_x >= 0)
		distancia = resta_x;
	else
		distancia = -resta_x;

	if(resta_y >= 0)
		distancia += resta_y;
	else
		distancia += -resta_y;

	return distancia;

}
