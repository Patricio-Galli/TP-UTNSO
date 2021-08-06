#include "utilidades.h"

char* crear_MD5(char caracter_llenado, int cantidad_caracteres) {
	log_info(logger, "Generando MD5");
	char caracter_string[2] = {caracter_llenado , '\0'};

	char* caracteres = string_new();
	string_append(&caracteres,"echo ");

	for(int i=0; i<cantidad_caracteres; i++)
		string_append(&caracteres,caracter_string);

	string_append(&caracteres, " | md5sum > archivoMD5_");
	string_append(&caracteres,caracter_string);

	int md5_result= system(caracteres);

	free(caracteres);

	if(md5_result >= 0) { //> o >= ?
		char* md5;
		char temp[12]="archivoMD5_";
		strcat(temp,caracter_string);//genero el archivo MD5 para cada recurso
		FILE* file = fopen(temp,"r");

		if(file == NULL)
			return NULL;

		fseek(file, 0, SEEK_END);
		long int size = 32;
		rewind(file);

		md5 = calloc(size + 1, 1);

		fread(md5,1,size,file);
		fclose(file);
		return md5;

	}
	else
		log_info(logger, "Fallo al calcular el MD5");

	return("ERROR DE MD5");
}

int roundUp(int a, int b){
	int R = a/b;
	int r = a % b;

	if(r > 0)
		R++;

	return R;
}

char* juntar_posiciones(int x, int y){
	char* posicion = string_new();
	char* x_str = string_itoa(x);
	char* y_str = string_itoa(y);

	string_append(&posicion, x_str);
	string_append(&posicion, "|");
	string_append(&posicion, y_str);

	free(x_str);
	free(y_str);

	return posicion;
}
