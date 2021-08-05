#include "blocks.h"

char* crear_MD5(char caracter_llenado, int cantidad_caracteres){
	log_info(logger, "Generando MD5");
	char caracter_string[2];
	caracter_string[0]=caracter_llenado;
	caracter_string[1]='\0';
	//char* bla=string_new();
	//string_append(&bla,"archivoMD5_");
	//string_append(&bla,caracter_string);
	//printf("%s\n",bla);
	//remove(bla);
	//free(bla);
	char* caracteres=string_new();
	string_append(&caracteres,"echo ");
	for(int i=0; i<cantidad_caracteres; i++){
		string_append(&caracteres,caracter_string);
	}
	string_append(&caracteres, " | md5sum > archivoMD5_");
	string_append(&caracteres,caracter_string);
	int md5_result= system(caracteres);
	free(caracteres);
	if(md5_result>=0){ //> o >= ?
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
/*
		    char* DIR_md5=string_new();
		    string_append(&DIR_md5,obtener_directorio("/archivoMD5_"));
		    string_append(&DIR_md5,caracter_string);
		    printf("%s",DIR_md5);
		    remove(DIR_md5);
		    */
		printf("%s \n",md5);
		fclose(file);
		//free(file);
		//free(temp);
		    return md5;

	}
	else
		log_info(logger, "Fallo al calcular el MD5");
	return("ERROR DE MD5");
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
	crear_metadata(DIR_metadata);

	t_config* metadata=config_create(DIR_metadata);
	int size_original=config_get_int_value(metadata,"SIZE");
	int bloques_max;
	int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
	//debo verificar si el ultimo bloque tiene espacio para escribir letras
	FILE* existe;
	existe=fopen(DIR_metadata,"rb");
	char** bloques_anteriores;
		if(existe!=NULL){
			char* bloques_anteriores_str=config_get_string_value(metadata,"BLOCKS");
			bloques_anteriores_str[0]=',';
			bloques_anteriores=string_split(bloques_anteriores_str,",");
			fclose(existe);
		}
		else{
			bloques_anteriores=config_get_array_value(metadata,"BLOCKS");
		}
	int size_final=size_original+cantidad_caracteres;
	if(size_original%block_size!=0){
		log_info(logger, "Se encontro lugar en el ultimo bloque");
		int aux=cantidad_caracteres;
		cantidad_caracteres=escribir_caracter_en_bloque(caracter_llenado, cantidad_caracteres,bloques_anteriores[cantidad_original_bloques-1],size_original);
		size_original+=aux-cantidad_caracteres;
	}

	bloques_max=roundUp(size_final,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia
	int bloques_faltantes=bloques_max-cantidad_original_bloques;
	if(bloques_faltantes>0){//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
		char* bloques_a_agregar= string_new();
		pthread_mutex_lock(&actualizar_bitmap);
		char* bloque_libre=proximo_bloque_libre();
		pthread_mutex_unlock(&actualizar_bitmap);

		string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres
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
		char* bloques_totales=string_new();
		string_append(&bloques_totales,"[");
		if(cantidad_original_bloques>0){
			char* temp=bloques_anteriores[1];
			temp[1]='\0';//elimino el ]
			string_append(&bloques_totales,temp);
			free(temp);
					for(int i=1;i<cantidad_original_bloques;i++){
						string_append(&bloques_totales,",");
						string_append(&bloques_totales,bloques_anteriores[i]);
					}

		}

		config_set_value(metadata,"SIZE",string_itoa(size_final));
		free(bloques_anteriores);
		if(cantidad_original_bloques==0){
			string_append(&bloques_totales,bloques_a_agregar);
		}else{
			string_append(&bloques_totales,",");
			string_append(&bloques_totales,bloques_a_agregar);
		}
		free(bloques_a_agregar);
		printf("Los bloques totales son: %s\n",bloques_totales);
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
			log_info(logger, "Buscando archivo de basura ya existentes");
			FILE* metadata;
				metadata=fopen(DIR_metadata,"rb");

				if(metadata!=NULL){
					log_info(logger, "Archivo existente encontrado");
					t_config* metadata=config_create(DIR_metadata);
					int cantidad_caracteres=config_get_int_value(metadata,"SIZE");
					int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
					//char* bloques_originales=config_get_string_value(metadata,"BLOCKS");
					char** bloques_originales=config_get_array_value(metadata,"BLOCKS");
					//bloques_originales[0]=','; //porque si no copia el [
					//char** buffer=string_split(bloques_originales,",");

					int temp;
					//char* temp1;
					for(int i=0;i<cantidad_original_bloques;i++){
						//temp1=buffer[i];
						//temp=temp1[0]-48;
						temp=atoi(bloques_originales[i]);
						cantidad_caracteres=borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,bloques_originales[i],cantidad_caracteres);
						pthread_mutex_lock(&actualizar_bitmap);
						bitarray_clean_bit(bitmap,atoi(bloques_originales[i]));
						pthread_mutex_unlock(&actualizar_bitmap);
						//free(temp1);
					}
					if(cantidad_caracteres>0){
						borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,string_itoa(temp-1),config_get_int_value(metadata,"SIZE"));
					}
					//free(buffer);
					free(bloques_originales);
					log_info(logger, "Se tiro la basura (se elimino el archivo)");
					remove(DIR_metadata);
					//close(metadata);
					return;
				}
				else{
					log_info(logger, "No existe el archivo de basura");
					return;
				}

		}
		crear_metadata(DIR_metadata);
		t_config* metadata=config_create(DIR_metadata);
		int size_original=config_get_int_value(metadata,"SIZE");
		if((size_original-cantidad_caracteres)<0){
			log_info(logger, "Se intentaron quitar mas caracteres de los existentes");
			cantidad_caracteres=size_original;
		}
		int size_final=size_final=MAX(0,(size_original-cantidad_caracteres));
		int bloques_reducidos;
			int cantidad_original_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
			bloques_reducidos=roundUp(size_final,block_size); //con los caracteres nuevos me fijo cuantos bloques ocuparia
			int bloques_sobrantes=cantidad_original_bloques-bloques_reducidos;
			if(bloques_sobrantes>0){//si hay mas bloques de los que necesito, debo sacar bloques}
							//char* bloques_originales=config_get_string_value(metadata,"BLOCKS");
									char** bloques_originales=config_get_array_value(metadata,"BLOCKS");
								printf("Los bloques originales son:");// para ver si anda bien
							for(int i=0;i<cantidad_original_bloques;i++){
								printf("%s",bloques_originales[i]);
								printf(",");
							}printf("\n");
							//bloques_originales[0]=','; //porque si no copia el [

							char* bloques_a_dejar=string_new();
							string_append(&bloques_a_dejar,"[");
							//char** buffer=string_split(bloques_originales,",");

								for(int i=0;i<bloques_reducidos;i++){
									string_append(&bloques_a_dejar,bloques_originales[i]);
									if(i+1==bloques_reducidos){;
										string_append(&bloques_a_dejar,"]");
									}
									else
										string_append(&bloques_a_dejar,",");
									}
								int temp;
								//char* temp1=malloc(sizeof(char*));
								for(int i=bloques_reducidos;i<cantidad_original_bloques;i++){
											//temp1=buffer[i];
											temp=atoi(bloques_originales[i]);
											int aux=cantidad_caracteres;
											cantidad_caracteres=borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,bloques_originales[i],size_original);
											size_original-=aux-cantidad_caracteres;
											pthread_mutex_lock(&actualizar_bitmap);
											bitarray_clean_bit(bitmap,atoi(bloques_originales[i]));
											pthread_mutex_unlock(&actualizar_bitmap);
																		}
								if(cantidad_caracteres>0){
									borrar_caracter_en_bloque(caracter_llenado,cantidad_caracteres,string_itoa(temp-1),size_original);
											}
				printf("los bloques a dejar son %s\n",bloques_a_dejar);
						//agregar los caracteres al bloque
				free(bloques_originales);
				config_set_value(metadata,"BLOCK_COUNT",string_itoa(bloques_reducidos));//Actualizo la cantidad de bloques
				config_set_value(metadata,"BLOCKS",bloques_a_dejar);
				free(bloques_a_dejar);
				//free(buffer);
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
	            	//pthread_mutex_lock(&actualizar_bitmap);
	                if(bitarray_test_bit(bitmap, i) == 0){
	                    bitarray_set_bit(bitmap,i);
	                    msync(bitmap->bitarray,bitmap_size,MS_SYNC); //uso semaforo?
	                   // pthread_mutex_unlock(&actualizar_bitmap);
	                    char* bloqueLibre= string_itoa(i);
	                    return bloqueLibre;
	                }
	}

	log_info(logger, "Todos los bloques estan ocupados");
	return NULL;
}
int escribir_caracter_en_bloque(char caracter_llenado,int cantidad_caracteres,char* bloque_libre,int size){
	int bloqueALlenar=atoi(bloque_libre);
	int caract_escritos=0;
	for(int caracteresEnBloque=size%block_size;caracteresEnBloque<block_size && cantidad_caracteres>0;caracteresEnBloque++){
				int desplazamiento=bloqueALlenar*block_size+caracteresEnBloque;
		pthread_mutex_lock(&actualizar_blocks);
				memcpy(blocks_copy+desplazamiento,string_from_format("%c",caracter_llenado),sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);
				caract_escritos++;
				cantidad_caracteres--;
	}
	printf("se escribieron %d\'%c\'\n",caract_escritos,caracter_llenado);
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
	//printf("caracteres en bloque %d\n",size%block_size);
	//printf("cantidad de caracteres: %d\n",cantidad_caracteres);
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

	printf("se guardaron %d caracteres\n",i);

	if(cantidad_caracteres==0){
		log_info(logger, "Se completo la escritura de caracteres");
	}
	else{
		log_info(logger, "Bloque completo");
	}
	return ultima_pos_mensaje;
}
int borrar_caracter_en_bloque(char caracter_llenado,int cantidad_caracteres,char* bloque_libre,int size){
	int bloqueALlenar=atoi(bloque_libre);
	int caracteresEnBloque;
	int caract_borrados=0;
	if(size%block_size==0){
		caracteresEnBloque=8;
	}
	else
		caracteresEnBloque=size%block_size;

	while(caracteresEnBloque>0 && cantidad_caracteres>0){
				int desplazamiento=bloqueALlenar*block_size+caracteresEnBloque;
		pthread_mutex_lock(&actualizar_blocks);
				memcpy(blocks_copy+desplazamiento,"\0",sizeof(char));
		pthread_mutex_unlock(&actualizar_blocks);
				caract_borrados++;
				cantidad_caracteres--;
		caracteresEnBloque--;
	}
	printf("se borraron %d\'%c\'\n",caract_borrados,caracter_llenado);
	if(cantidad_caracteres==0){
		log_info(logger, "Se completo el borrado de caracteres");
	}
	else
		log_info(logger, "Bloque vacio");
	return cantidad_caracteres;
}
//se puede hacer en un switch
void actualizar_posicion(tripulante* tripulante, int x_nueva, int y_nueva,char* DIR_Bit_Tripulante){
	int x_anterior=tripulante->posicion_x;
	int y_anterior=tripulante->posicion_y;

	char* mensaje=string_new();
	string_append(&mensaje,"Se mueve de ");
	string_append(&mensaje,juntar_posiciones(x_anterior,y_anterior));
	string_append(&mensaje," a ");
	string_append(&mensaje,juntar_posiciones(x_nueva,y_nueva));
	string_append(&mensaje,".");

	escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);
}
void comienza_tarea(char* tarea,char* DIR_Bit_Tripulante){

	char* mensaje=string_new();
		string_append(&mensaje,"Comienza ejecucion de tarea ");
		string_append(&mensaje,tarea);
		string_append(&mensaje,".");
		escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);

}
void finaliza_tarea(char* tarea,char* DIR_Bit_Tripulante){
	char* mensaje=string_new();
		string_append(&mensaje,"Finaliza la tarea ");
		string_append(&mensaje,tarea);
		string_append(&mensaje,".");
		escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);

}
void inicio_sabotaje(char* DIR_Bit_Tripulante){
	char* mensaje=string_new();
		string_append(&mensaje,"Se corre en panico hacia la ubicacion del sabotaje.");
		escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);
}
void fin_sabotaje(char* DIR_Bit_Tripulante){
	char* mensaje=string_new();
		string_append(&mensaje,"Se resuelve el sabotaje.");
		escribir_mensaje_en_bitacora(mensaje,DIR_Bit_Tripulante);
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
void escribir_mensaje_en_bitacora(char* mensaje, char* DIR_Bit_Tripulante){
	int cantidad_caracteres=string_length(mensaje);
	printf("mensaje %s\n",mensaje);
	t_config* metadata=config_create(DIR_Bit_Tripulante);
	int size_original=config_get_int_value(metadata,"SIZE");
	int size=size_original;
	char** bloques_anteriores=config_get_array_value(metadata,"BLOCKS");
	int cantidad_original_bloques=roundUp(size,block_size);
	int ultima_pos_mensaje=0;
	if(size%block_size!=0){
		log_info(logger, "Se encontro lugar en el ultimo bloque");
			ultima_pos_mensaje=escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje,bloques_anteriores[cantidad_original_bloques-1],size);
		}
	int cant_anterior_bloques=roundUp(size,block_size);
		size+=cantidad_caracteres; //sumo todos los bytes

		int bloques_max;
		bloques_max=roundUp(size,block_size);
		int bloques_faltantes=(bloques_max-cant_anterior_bloques);

			if(bloques_faltantes>0){//si ocupo mas bloques de los que tengo asignados, debo agregar mas bloques
				char* bloques_a_agregar= string_new();
				pthread_mutex_lock(&actualizar_bitmap);
				char* bloque_libre=proximo_bloque_libre();
				pthread_mutex_unlock(&actualizar_bitmap);
				string_append(&bloques_a_agregar,bloque_libre);//agrego el proximo bloque disponible que va a almacenar los caracteres
				ultima_pos_mensaje=escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje,bloque_libre,size_original+ultima_pos_mensaje);
				free(bloque_libre);
				bloques_faltantes--;
			 //si siguen faltando agregar bloques los agrego
					for(int i=0;i<bloques_faltantes;i++){
						pthread_mutex_lock(&actualizar_bitmap);
						bloque_libre=proximo_bloque_libre();
						pthread_mutex_unlock(&actualizar_bitmap);
						string_append(&bloques_a_agregar,",");
						string_append(&bloques_a_agregar,bloque_libre);
						ultima_pos_mensaje=escribir_caracter_en_bitacora(mensaje, ultima_pos_mensaje,bloque_libre,size_original+ultima_pos_mensaje);
						free(bloque_libre);

						}
				string_append(&bloques_a_agregar,"]");
				printf("Los bloques a agregar son: %s\n",bloques_a_agregar);

						char* bloques_totales=string_new();
						string_append(&bloques_totales,"[");
						if(cant_anterior_bloques>0){
							string_append(&bloques_totales,bloques_anteriores[0]);
									for(int i=1;i<cant_anterior_bloques;i++){
										string_append(&bloques_totales,",");
										string_append(&bloques_totales,bloques_anteriores[i]);
									}
							string_append(&bloques_totales,",");
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
