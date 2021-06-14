#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <commons/string.h>

#include <commons/string.h>
#include <stdlib.h>

#include "discordiador.h"


typedef enum{
	INICIAR_PATOTA,
	LISTAR_TRIPULANTES,
	EXPULSAR_TRIPULANTE,
	INICIAR_PLANIFICACION,
	PAUSAR_PLANIFICACION,
	OBTENER_BITACORA,
	EXIT_DISCORDIADOR,
	ERROR
}command_code;

char* leer_consola();
command_code mapStringToEnum(char *string);
char* seleccionar_funcion(char* buffer);

#endif /* CONSOLA_H_ */
