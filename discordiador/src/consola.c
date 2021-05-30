#include "consola.h"

char** leer_consola(){
	char* linea;
	char** input;

	linea = readline(">");

	if(linea)
		add_history(linea);

	input = string_split(linea," ");
	free(linea);

	return input;
}
