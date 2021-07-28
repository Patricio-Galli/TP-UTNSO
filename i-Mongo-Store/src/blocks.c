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
	int size_original=config_get_int_value(metadata,"SIZE");
	int size_final=size_original+cantidad_caracteres;
	//size_final+=cantidad_caracteres; //sumo todos los bytes


	int bloques_max;
	int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
	bloques_max=roundUp(size_final,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia

	int bloques_faltantes=bloques_max-cantidad_original_bloques;
	if(bloques_faltantes>0){//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
		char* bloques_a_agregar= string_new();
		pthread_mutex_lock(&actualizar_bitmap);
		char* bloque_libre=proximo_bloque_libre();
		pthread_mutex_unlock(&actualizar_bitmap);
		string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres
		//semaforo
		cantidad_caracteres=escribir_caracter_en_bloque(caracter_llenado, cantidad_caracteres,bloque_libre,size_original);
		free(bloque_libre);
		bloques_faltantes--;

	 //si siguen faltando agregar bloques los agrego
			for(int i=0;i<bloques_faltantes;i++){
				pthread_mutex_lock(&actualizar_bitmap);
				bloque_libre=proximo_bloque_libre();
				pthread_mutex_unlock(&actualizar_bitmap);
				string_append(&bloques_a_agregar,",");
				string_append(&bloques_a_agregar,bloque_libre);
				cantidad_caracteres=escribir_caracter_en_bloque(caracter_llenado, cantidad_caracteres,bloque_libre,size_original);
				free(bloque_libre);
				}
		string_append(&bloques_a_agregar,"]");
		printf("Los bloques a agregar son: %s\n",bloques_a_agregar);
				//agregar los caracteres al bloque

	//junto los bloques viejos con los nuevos y lo guardo en la metadata
		char** bloques_anteriores=config_get_array_value(metadata,"BLOCKS");
		char* bloques_totales=string_new();
		string_append(&bloques_totales,"[");
		if(cantidad_original_bloques>0){
			string_append(&bloques_totales,bloques_anteriores[0]);
					for(int i=1;i<cantidad_original_bloques;i++){
						string_append(&bloques_totales,",");
						string_append(&bloques_totales,bloques_anteriores[i]);
					}
			string_append(&bloques_a_agregar,",");
		}
		config_set_value(metadata,"SIZE",string_itoa(size_final));
		free(bloques_anteriores);
			string_append(&bloques_totales,bloques_a_agregar);
		free(bloques_a_agregar);
		config_set_value(metadata,"BLOCKS",bloques_totales);
		free(bloques_totales);
		config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_max));//Actualizo la cantidad de bloques
	}
	//actualizo el MD5
		char* MD5_nuevo=crear_MD5(caracter_llenado,size_final);
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
		int size_original=config_get_int_value(metadata,"SIZE");
		int size_final=size_final=MAX(0,(size_original-cantidad_caracteres));
		//size_final-=cantidad_caracteres; //sumo todos los bytes


		int bloques_reducidos;
			int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
			int block_size=config_get_int_value(config,"BLOCK_SIZE");
			bloques_reducidos=roundUp(size_final,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia

			int bloques_sobrantes=cantidad_original_bloques-bloques_reducidos;
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
							cantidad_caracteres=borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,bloques_originales[i],size_original);
							bitarray_clean_bit(bitmap,temp);
							if(cantidad_caracteres>0){
								int a=size_original;
								a-=block_size;
								config_set_value(metadata,"SIZE",string_itoa(a));
							}
							}
					if(cantidad_caracteres>0){
						borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,string_itoa(temp-1),config_get_int_value(metadata,"SIZE"));
					}
				printf("los bloques a dejar son %s\n",bloques_a_dejar);
						//agregar los caracteres al bloque


				free(bloques_originales);
				config_set_value(metadata,"BLOCKS",bloques_a_dejar);
				free(bloques_a_dejar);
				config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_reducidos));//Actualizo la cantidad de bloques
			}
			config_set_value(metadata,"SIZE",string_itoa(size_final));
			//actualizo el MD5
				char* MD5_nuevo=crear_MD5(caracter_llenado,size_final);
				config_set_value(metadata,"MD5_ARCHIVO",MD5_nuevo);

				free(MD5_nuevo);
				config_save(metadata);

}
char* proximo_bloque_libre(){// busca el prox libre y lo pone en 1
	int bitmap_size=(blocks_amount/8);
	for(int i=0; i<(blocks_amount); i++) {
	            	//pthread_mutex_lock(&sincro_bitmap); //TODO LA SECCION CRITICA PODRIA O DEBERIA SER MAS CORT
	                if(bitarray_test_bit(bitmap, i) == 0){
	                    bitarray_set_bit(bitmap,i);
	                    msync(bitmap->bitarray,bitmap_size,MS_SYNC); //uso semaforo?
	                    char* bloqueLibre= string_itoa(i);
	                    return bloqueLibre;
	                }
	}
	//return ERROR_BITMAP_LLENO;
	return NULL;
}
int escribir_caracter_en_bloque(char caracter_llenado,int cantidad_caracteres,char* bloque_libre,int size){
	int bloqueALlenar=atoi(bloque_libre);
	for(int caracteresEnBloque=size%block_size;caracteresEnBloque<block_size && cantidad_caracteres>0;caracteresEnBloque++){
				int desplazamiento=bloqueALlenar*block_size+caracteresEnBloque;
		pthread_mutex_lock(&actualizar_blocks);
				memcpy(blocks_copy+desplazamiento,string_from_format("%c",caracter_llenado),sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);
				printf("se escribio una %c\n",caracter_llenado);
				cantidad_caracteres--;
	}
	if(cantidad_caracteres==0){
		log_info(logger, "Se completo la escritura de caracteres");
	}
	else
		log_info(logger, "Bloque completo");
	return cantidad_caracteres;
}
int escribir_caracter_en_bitacora(char* mensaje,int ultima_pos_mensaje,char* bloque_libre,int size){
	int bloqueALlenar=atoi(bloque_libre);
	int cantidad_caracteres=string_length(mensaje)-ultima_pos_mensaje; //si son 20 carac el msg,y escribi 3carac, estoy en la posicion 2, y me faltan 17 caracteres
	int i=0;
	printf("Mensaje escrito:\"");
	for(int caracteresEnBloque=size%block_size;caracteresEnBloque<block_size && cantidad_caracteres>0;caracteresEnBloque++){
				int desplazamiento=bloqueALlenar*block_size+caracteresEnBloque;
		pthread_mutex_lock(&actualizar_blocks);
				memcpy(blocks_copy+desplazamiento,string_from_format("%c",mensaje[ultima_pos_mensaje]),sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);
				printf("%c",mensaje[ultima_pos_mensaje]);
				ultima_pos_mensaje++;
				cantidad_caracteres--;
				i++;
	}
	printf("\"\n");
	printf("se guardaron %d caracteres\n",i);
	if(cantidad_caracteres==0){
		log_info(logger, "Se completo la escritura de caracteres");
	}
	else
		log_info(logger, "Bloque completo");
	return ultima_pos_mensaje;
}
int borrar_caracter_en_bloque(char caracter_llenado,int cantidad_caracteres,char* bloque_libre,int size){
	int bloqueALlenar=atoi(bloque_libre);
	int caracteresEnBloque;
	if(size%block_size==0){
		caracteresEnBloque=8;
	}
	else
		caracteresEnBloque=size%block_size;
	for(caracteresEnBloque;caracteresEnBloque>0 && cantidad_caracteres>0;caracteresEnBloque--){
				int desplazamiento=bloqueALlenar*block_size+caracteresEnBloque;
		pthread_mutex_lock(&actualizar_blocks);
				memcpy(blocks_copy+desplazamiento,"\0",sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);
				printf("se borro una %c\n",caracter_llenado);
				cantidad_caracteres--;
	}
	if(cantidad_caracteres==0){
		log_info(logger, "Se completo el borrado de caracteres");
	}
	else
		log_info(logger, "Bloque vacio");
	return cantidad_caracteres;
}

void actualizar_posicion(tripulante* tripulante, int x_nueva, int y_nueva,char* DIR_Bit_Tripulante){
	int x_anterior=tripulante->posicion_x;
	int y_anterior=tripulante->posicion_y;

	char* mensaje=string_new();
	string_append(&mensaje,"Se mueve de ");
	string_append(&mensaje,juntar_posiciones(x_anterior,y_anterior));
	string_append(&mensaje," a ");
	string_append(&mensaje,juntar_posiciones(x_nueva,y_nueva));
	int cantidad_caracteres=string_length(mensaje); //lleva el &?
	printf("total caracteres %d\n",cantidad_caracteres);
	printf("mensaje %s\n",mensaje);


	t_config* metadata=config_create(DIR_Bit_Tripulante);
	int size=config_get_int_value(metadata,"SIZE");
	int cant_anterior_bloques=roundUp(size,block_size);
		size+=cantidad_caracteres; //sumo todos los bytes

		int bloques_max;
		bloques_max=roundUp(size,block_size);
		int ultima_pos_mensaje;
		int bloques_faltantes=(bloques_max-cant_anterior_bloques);

			if(bloques_faltantes>0){//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
				char* bloques_a_agregar= string_new();
				pthread_mutex_lock(&actualizar_bitmap);
				char* bloque_libre=proximo_bloque_libre();
				pthread_mutex_unlock(&actualizar_bitmap);
				string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres

				ultima_pos_mensaje=escribir_caracter_en_bitacora(mensaje, 0,bloque_libre,config_get_int_value(metadata,"SIZE"));
				free(bloque_libre);
				bloques_faltantes--;
			 //si siguen faltando agregar bloques los agrego
					for(int i=0;i<bloques_faltantes;i++){
						pthread_mutex_lock(&actualizar_bitmap);
						bloque_libre=proximo_bloque_libre();
						pthread_mutex_lock(&actualizar_bitmap);
						string_append(&bloques_a_agregar,",");
						string_append(&bloques_a_agregar,bloque_libre);
						ultima_pos_mensaje=escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje,bloque_libre,config_get_int_value(metadata,"SIZE"));
						free(bloque_libre);
						}
				string_append(&bloques_a_agregar,"]");
				printf("Los bloques a agregar son: %s\n",bloques_a_agregar);
				char** bloques_anteriores=config_get_array_value(metadata,"BLOCKS");
						char* bloques_totales=string_new();
						string_append(&bloques_totales,"[");
						if(cant_anterior_bloques>0){
							string_append(&bloques_totales,bloques_anteriores[0]);
									for(int i=1;i<cant_anterior_bloques;i++){
										string_append(&bloques_totales,",");
										string_append(&bloques_totales,bloques_anteriores[i]);
									}
							string_append(&bloques_a_agregar,",");
						}config_save(metadata);
						config_set_value(metadata,"SIZE",string_itoa(size));
						free(bloques_anteriores);
							string_append(&bloques_totales,bloques_a_agregar);
						free(bloques_a_agregar);
						config_set_value(metadata,"BLOCKS",bloques_totales);
						free(bloques_totales);
						//config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_max));//Actualizo la cantidad de bloques
			config_save(metadata);
}
}
char* juntar_posiciones(int x, int y){
	char* posicion=string_new();
	string_append(&posicion,string_itoa(x));
	string_append(&posicion,"|");
	string_append(&posicion,string_itoa(y));
	return posicion;
}


int roundUp(int a, int b){
	int R=a/b;
	int r=a%b;
	if(r>0){
		R++;
	}
	return R;
}
