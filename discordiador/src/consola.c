#include "consola.h"

void iniciarConsola(){

	char * linea;
	while(1){
		linea = readline(">");
		if(!strncmp(linea,"exit",4)) {
			free(linea);
			break;
		}
		if(linea)
			add_history(linea);

		char** input = string_split(linea," ");
		command_code command = mapStringToEnum(input[0]);
		char** params = &input[1];
		switch(command){
			case INICIAR_PATOTA:
				printf("INICIAR PATOTA RUNNING");
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
		}
		// SOLO PARA TESTEAR:
		for (char* iterator = *params; iterator; iterator=*++params){
			printf("PARAMETRO: %s",iterator);
		}
		free(*input);
		free(input);
		free(linea);
	}

}

command_code mapStringToEnum(char *string){
	if(strncmp(string,"INICIAR_PATOTA",strlen(string)) == 0){
		return INICIAR_PATOTA;
	}
	else if(strncmp(string,"LISTAR_TRIPULANTES",strlen(string)) == 0){
		return LISTAR_TRIPULANTES;
	}
	else if(strncmp(string,"EXPULSAR_TRIPULANTE",strlen(string)) == 0){
		return EXPULSAR_TRIPULANTE;
	}
	else if(strncmp(string,"INICIAR_PLANIFICACION",strlen(string)) == 0){
		return INICIAR_PLANIFICACION;
	}
	else if(strncmp(string,"PAUSAR_PLANIFICACION",strlen(string)) == 0){
		return PAUSAR_PLANIFICACION;
	}
	else if(strncmp(string,"OBTENER_BITACORA",strlen(string)) == 0){
			return OBTENER_BITACORA;
	}
	return -1;

}
