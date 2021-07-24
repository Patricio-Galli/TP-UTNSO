#include "blocks.h"

char* crear_MD5(char caracter_llenado, int cantidad_caracteres){
	log_info(logger, "Generando MD5");
	char caracter_string[2];
	caracter_string[0]=caracter_llenado;
	caracter_string[1]='\0';
	char* caracteres=string_new();
	string_append(&caracteres,"echo ");
	for(int i=0; i<cantidad_caracteres; i++){
		string_append(&caracteres,caracter_string);
	}
	string_append(&caracteres, " | md5sum > archivoMD5_");
	string_append(&caracteres,caracter_string);
	int md5= system(caracteres);
	free(caracteres);
	if(md5>=0){ //> o >= ?
		printf("MD5 calculado: ");
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
		log_info(logger, "Fallo al calcular el MD5");
	return(-1);
}
void sumar_caracteres(char caracter_llenado, int cantidad_caracteres){
	char DIR_metadata[90];
	if(caracter_llenado=='O'){
		strcpy(DIR_metadata,obtener_directorio("/Files/Oxigeno.ims"));
	}
	else if(caracter_llenado=='C'){
		strcpy(DIR_metadata,obtener_directorio("/Files/Comida.ims"));
	}
	else{
		strcpy(DIR_metadata,obtener_directorio("/Files/Basura.ims"));
	}

	t_config* metadata=config_create(DIR_metadata);

	int size=config_get_int_value(metadata,"SIZE");
	size+=cantidad_caracteres; //sumo todos los bytes
	config_set_value(metadata,"SIZE",string_itoa(size));

	int bloques_max;
	int block_size=config_get_int_value(config,"BLOCK_SIZE");
	bloques_max=roundUp(size,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia

	int bloques_faltantes=bloques_max-(config_get_int_value(metadata,"BLOCK_COUNT"));
	if(bloques_faltantes>0){//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
		char* bloques_a_agregar= string_new();
		char* bloque_libre=proximo_bloque_libre();
		string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres
		free(bloque_libre);
		bloques_faltantes--;

	 //si siguen faltando agregar bloques los agrego
			for(int i=0;i<bloques_faltantes;i++){
				bloque_libre=proximo_bloque_libre();
				string_append(&bloques_a_agregar,",");
				string_append(&bloques_a_agregar,bloque_libre);
				free(bloque_libre);
				}
		string_append(&bloques_a_agregar,"]");
		printf("Los bloques a agregar son: %s\n",bloques_a_agregar);
				//agregar los caracteres al bloque

	//junto los bloques viejos con los nuevos y lo guardo en la metadata
		char** bloques_anteriores=config_get_array_value(metadata,"BLOCKS");
		char* bloques_totales=string_new();
		string_append(&bloques_totales,"[");
		if(config_get_int_value(metadata,"BLOCK_COUNT")>0){
			string_append(&bloques_totales,bloques_anteriores[0]);
					for(int i=1;i<config_get_int_value(metadata,"BLOCK_COUNT");i++){
						string_append(&bloques_totales,",");
						string_append(&bloques_totales,bloques_anteriores[i]);
					}
			string_append(&bloques_a_agregar,",");
		}
		free(bloques_anteriores);
			string_append(&bloques_totales,bloques_a_agregar);
		free(bloques_a_agregar);
		config_set_value(metadata,"BLOCKS",bloques_totales);
		free(bloques_totales);
		config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_max));//Actualizo la cantidad de bloques
	}
	//actualizo el MD5
		char* MD5_nuevo=crear_MD5(caracter_llenado,size);
		config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);

		free(MD5_nuevo);
		config_save(metadata);
}

void quitar_caracteres(char caracter_llenado, int cantidad_caracteres){
	char DIR_metadata[90];
		if(caracter_llenado=='O'){
			strcpy(DIR_metadata,obtener_directorio("/Files/Oxigeno.ims"));
		}
		else if(caracter_llenado=='C'){
			strcpy(DIR_metadata,obtener_directorio("/Files/Comida.ims"));
		}
		else{
			strcpy(DIR_metadata,obtener_directorio("/Files/Basura.ims"));
		}

		t_config* metadata=config_create(DIR_metadata);
		int size=config_get_int_value(metadata,"SIZE");
		size-=cantidad_caracteres; //sumo todos los bytes
		config_set_value(metadata,"SIZE",string_itoa(size));
		int bloques_reducidos;
			int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
			int block_size=config_get_int_value(config,"BLOCK_SIZE");
			bloques_reducidos=roundUp(size,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia

			int bloques_sobrantes=(config_get_int_value(metadata,"BLOCK_COUNT"))-bloques_reducidos;
			if(bloques_sobrantes>0){//si hay mas bloques de los que necesito, debo sacar bloques}
				char** bloques_originales=config_get_array_value(metadata,"BLOCKS");
				char* bloques_a_dejar= string_new();
				string_append(&bloques_a_dejar,"[");

					for(int i=0;i<(cantidad_original_bloques-bloques_sobrantes);i++){
						string_append(&bloques_a_dejar,bloques_originales[i]);
						if((i+1)==(cantidad_original_bloques-bloques_sobrantes)){
							string_append(&bloques_a_dejar,"]");
						}
						else
						string_append(&bloques_a_dejar,",");
						}
					int temp;
				//actualizo el bitmap
					for(int i=(cantidad_original_bloques-bloques_sobrantes);i<cantidad_original_bloques;i++){
							temp=atoi(bloques_originales[i]);
							bitarray_clean_bit(bitmap,temp);

							}
				printf("los bloques a dejar son %s\n",bloques_a_dejar);
						//agregar los caracteres al bloque


				free(bloques_originales);
				config_set_value(metadata,"BLOCKS",bloques_a_dejar);
				free(bloques_a_dejar);
				config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_reducidos));//Actualizo la cantidad de bloques
			}
			//actualizo el MD5
				char* MD5_nuevo=crear_MD5(caracter_llenado,size);
				config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);

				free(MD5_nuevo);
				config_save(metadata);

}
char* proximo_bloque_libre(){// busca el prox libre y lo pone en 1
	int bitmap_size=(config_get_int_value(config,"BLOCKS_AMOUNT")/8);
	for(int i=0; i<(config_get_int_value(config,"BLOCKS_AMOUNT")); i++) {
	            	//pthread_mutex_lock(&sincro_bitmap); //TODO LA SECCION CRITICA PODRIA O DEBERIA SER MAS CORT
	                if(bitarray_test_bit(bitmap, i) == 0){
	                    bitarray_set_bit(bitmap,i);
	                    msync(bitmap->bitarray,bitmap_size,MS_SYNC); //uso semaforo?
	                    char* bloqueLibre= string_itoa(i);
	                    return bloqueLibre;
	                }
	}
}
int roundUp(int a, int b){
	int R=a/b;
	int r=a%b;
	if(r>0){
		R++;
	}
	return R;
}
