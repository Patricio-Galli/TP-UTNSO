#include "blocks.h"


char* crear_MD5(char caracter_llenado, int cantidad_caracteres){
	char caracter_string[2];
	caracter_string[0]=caracter_llenado;
	caracter_string[1]='\0';
	char* caracteres=string_new();
	string_append(&caracteres,"echo ");
	printf("%s\n",caracteres);
	for(int i=0; i<cantidad_caracteres; i++){
		string_append(&caracteres,caracter_string);
	}
	string_append(&caracteres, " | md5sum > archivoMD5_");
	string_append(&caracteres,caracter_string);
	int md5= system(caracteres);
	free(caracteres);
	if(md5>=0){ //> o >= ?
		printf("MD5 calculado\n");
		char* md5;
		char temp[12]="archivoMD5_";
		strcat(temp,caracter_string);//genero el archivo MD5 para cada recurso
		FILE* file = fopen(temp,"r");
		    if(file == NULL)
		    {
		        return NULL;
		    }

		    fseek(file, 0, SEEK_END);
		    long int size = 32;
		    rewind(file);

		    md5= calloc(size + 1, 1);

		    fread(md5,1,size,file);

		printf("%s \n",md5);
		    return md5;

	}
	else
		printf("Fallo al calcular el MD5\n");
	return(-1);
}
void agregar_Caracteres(char caracter_llenado, int cantidad_caracteres){
	if(caracter_llenado=='O'){
		char DIR_metadata[90];
		strcpy(DIR_metadata,obtener_directorio("/Files/Oxigeno.ims"));
		t_config* metadata=config_create(DIR_metadata);
	}
	else if(caracter_llenado=='C'){
		char DIR_metadata[90];
		strcpy(DIR_metadata,obtener_directorio("/Files/Comida.ims"));
		t_config* metadata=config_create(DIR_metadata);
	}
	else{
		char DIR_metadata[90];
		strcpy(DIR_metadata,obtener_directorio("/Files/Basura.ims"));
		t_config* metadata=config_create(DIR_metadata);
	}


}/*
char* proximo_bloque_libre(){

	for(int i=0; i<(config_get_int_value(config,"BLOCKS_AMOUNT")); i++) {
	            	//pthread_mutex_lock(&sincro_bitmap); //TODO LA SECCION CRITICA PODRIA O DEBERIA SER MAS CORT
	                if(bitarray_test_bit(bitmap, i) == 0){
	                    int numeroBloque = i;
	                    char* bloqueLibre= string_itoa(i);
	                    return bloqueLibre;

}}*/
