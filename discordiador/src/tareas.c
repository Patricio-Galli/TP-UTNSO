#include "tareas.h"

void ejecutar(char* input, tripulante* tripulante) {
	char** buffer = string_split(input, ";");
	char** comando_tarea = string_split(buffer[0], " ");
	bool tarea_concretada = false;

	tareas tarea = stringToEnum(comando_tarea[0]);

	//todo  avisar al mongo que se empezo a ejecutar esa tarea

	int pos_x = atoi(comando_tarea[1]);
	int pos_y = atoi(comando_tarea[2]);

	while(tripulante->posicion[0] != pos_x && tripulante->estado == RUNNING) {
		tripulante->posicion[0]++;
		//todo avisar a ram
		//todo avisar a mongo
		tripulante->contador_ciclos++;
	}
	while(tripulante->posicion[1] != pos_y && tripulante->estado == RUNNING) {
			tripulante->posicion[1]++;
			//todo avisar a ram
			//todo avisar a mongo
			tripulante->contador_ciclos++;
	}

	if(tripulante->estado == RUNNING) {
		if(tarea == ESPERAR) {
			int i = 0;
			int tiempo_espera = atoi(buffer[3]);
			while(i < tiempo_espera && tripulante->estado == RUNNING) {
				tripulante->contador_ciclos++;
				i++;
				sleep(ciclo_CPU);
			}
		}
		else {
			tripulante->contador_ciclos++;
			switch(tarea){
				case GENERAR_OXIGENO:
					//activar_io
					//generar_oxigeno
					break;
				case CONSUMIR_OXIGENO:
					break;
				case GENERAR_COMIDA:
					break;
				case CONSUMIR_COMIDA:
					break;
				case GENERAR_BASURA:
					break;
				case DESCARTAR_BASURA:
					break;
				case ESPERAR:
					break;
			}
		}
	}

	if(tarea_concretada) {
		//todo  avisar al mongo que se termino de ejecutar la tarea
	}
}

// avisar al mongo que se termino de ejecutar esa tarea

tareas stringToEnum(char *string){
	char* listaDeStrings[]={"GENERAR_OXIGENO", "CONSUMIR_OXIGENO", "GENERAR_COMIDA", "CONSUMIR_COMIDA", "GENERAR_BASURA", "DESCARTAR_BASURA"};

	for(int i=0;i<6;i++){
		if(!strcasecmp(string,listaDeStrings[i])) {
			free(string);
			return i;
		}
	}
	free(string);
	return ESPERAR;
}
