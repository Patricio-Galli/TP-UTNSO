#include "imongostore.h"




int tripu, pat = 0;
int main(void)
{
	logger=log_create("imongostore.log", "I-MONGO-STORE", 1, LOG_LEVEL_DEBUG);
	config=config_create("imongostore.config");
	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");

	log_info(logger, "toy aqui");

		mkdir(punto_montaje,0755); //se crea el directorio /FileSystem/
		printf("el punto de montaje es %s\n",punto_montaje);

		//creo el superBloque (si no esta creado)
			char DIR_superBloque[100]; //1
			//char* DIR_superBloque=malloc(sizeof(punto_montaje));
			//strcpy(DIR_superBloque,punto_montaje);
			//strcat(DIR_superBloque,"/superBloque.ims");

			//sprintf(DIR_superBloque,"/superBloque.ims");
			//sprintf(DIR_superBloque,"/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/superBloque.ims");
			strcpy(DIR_superBloque,obtener_directorio("/superBloque.ims")); //1
			printf("el directorio del superbloque es %s\n",DIR_superBloque);
			block_size=config_get_long_value(config, "BLOCK_SIZE");
			blocks_amount=config_get_long_value(config, "BLOCKS_AMOUNT");
			crear_superBloque(DIR_superBloque);


			char DIR_Blocks[100];
			strcpy(DIR_Blocks,obtener_directorio("/Blocks.ims"));
			printf("el directorio del blocks es %s\n",DIR_Blocks);
			crear_blocks(DIR_Blocks);

		char DIR_metadata[150];
		generar_directorio("/Files");
		generar_directorio("/Files/Bitacoras");
		strcpy(DIR_metadata,obtener_directorio("/Files/Oxigeno.ims"));
		crear_metadata(DIR_metadata);

		strcpy(DIR_metadata,obtener_directorio("/Files/Comida.ims"));
		crear_metadata(DIR_metadata);

		strcpy(DIR_metadata,obtener_directorio("/Files/Basura.ims"));
		crear_metadata(DIR_metadata);

		int id_tripulante=1;
		strcpy(DIR_metadata,obtener_directorio("/Files/Bitacoras/Tripulante"));
		strcat(DIR_metadata,string_itoa(id_tripulante));
		strcat(DIR_metadata,".ims");
		crear_bitacora(DIR_metadata);

		imprimir_bitmap(bitmap);
		printf("sumar 20 oxigenos \n");
		sumar_caracteres('O',20);
		imprimir_bitmap(bitmap);
		printf("consumir 8 oxigenos \n");
		quitar_caracteres('O',8);
		imprimir_bitmap(bitmap);

		printf("sumar 24 comidas \n");
		sumar_caracteres('C',24);
		imprimir_bitmap(bitmap);
		printf("consumir 12 Comidas \n");
		quitar_caracteres('C',12);
		imprimir_bitmap(bitmap);

		tripulante* temp2=malloc(sizeof(tripulante));
		temp2->id_trip=1;temp2->posicion_x=1;temp2->posicion_y=2;temp2->id_patota=1;temp2->socket_discord=2;
		actualizar_posicion(temp2,3,4,"/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/Files/Bitacoras/Tripulante1.ims");

		imprimir_bitmap(bitmap);
	log_info(logger, "Creando conexiones");


}

char* crear_superBloque(char* DIR_superBloque){
	log_info(logger, "Verificando existencia SuperBloque");
	FILE* existe= fopen(DIR_superBloque,"r");
	if(existe != NULL){
		log_info(logger, "SuperBloque ya existe");
	}
	else
		log_info(logger, "SuperBloque no existe, creandolo.");
	printf("el tamaño del bloque es %d y la cant de bloques es %d \n",block_size,blocks_amount);
	int bitmap_size=(blocks_amount/8);

	printf("el directorio del superBloque es %s\n",DIR_superBloque);
	int fp = open(DIR_superBloque, O_CREAT | O_RDWR, 0664);

	if(fp==-1){
		log_info(logger, "No se pudo abrir/generar el archivo");
		exit(-1);
	}
	ftruncate(fp,sizeof(uint32_t)*2+bitmap_size);
	void* superBloque = mmap(NULL, sizeof(uint32_t)*2 + bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
//genero todo en 0 el bitmap
	if (superBloque == MAP_FAILED) {
				log_error(logger, "Error al mapear el SuperBloque");
				close(fp);
				return (-1);
			}

	bitmap = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), bitmap_size, MSB_FIRST);
	for(int i=0;i<bitarray_get_max_bit(bitmap);i++){
		bitarray_clean_bit(bitmap,i);
	}
	void* prueba=malloc(4);
	memcpy(prueba,&block_size,sizeof(uint32_t));
	memcpy(superBloque,prueba,sizeof(uint32_t));
	memcpy(prueba,&blocks_amount,sizeof(uint32_t));
	memcpy(superBloque+sizeof(uint32_t),prueba,sizeof(uint32_t));
	msync(bitmap->bitarray,bitmap_size,MS_SYNC);
	msync(superBloque, sizeof(uint32_t)*2 + bitmap_size, MS_SYNC);

	printf("se escribio el archivo\n");
	close(fp);
	free(prueba);
	char* puntero_SuperBloque=DIR_superBloque;;
	log_info(logger, "SuperBloque Generado");

	return puntero_SuperBloque;
}
char* crear_blocks(char* DIR_blocks){
	log_info(logger, "Verificando existencia Blocks");
		FILE* existe= fopen(DIR_blocks,"r");
		if(existe != NULL){
			log_info(logger, "Blocks ya existe");
		}else
			log_info(logger, "Blocks no existe, creandolo");
	int size=block_size*blocks_amount;
	int fp = open(DIR_blocks, O_CREAT | O_RDWR, 0666);
	if (fp == -1){
		log_info(logger, "No se pudo abrir/generar el archivo");
		exit(-1);
	}

	ftruncate(fp,size);
	//void* superBloque = mmap(NULL, sizeof(uint32_t)*2 + bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
	void* blocks = mmap(NULL, size, PROT_READ | PROT_WRITE,MAP_SHARED, fp, 0);
	if (blocks == MAP_FAILED){
		log_info(logger, "Error al mapear Blocks");

		exit(-1);
	}
	log_info(logger, "Blocks Generado");
	blocks_copy= malloc(size);
	close(fp);
	return(blocks);
}
void uso_blocks(void* blocks){//se deberia encargar un hilo de esto?
	int size=block_size*blocks_amount;
	blocks_copy= malloc(size);
			sleep(config_get_int_value(config, "TIEMPO_SINCRONIZACION"));
			pthread_mutex_lock(&actualizar_blocks);
			memcpy(blocks_copy,blocks,size);
			log_info(logger, "Se realizo copia de blocks");
			pthread_mutex_unlock(&actualizar_blocks);
			msync(blocks,(size), MS_SYNC);
			log_info(logger, "Se sincronizo el blocks");
}
void crear_metadata(char* DIR_metadata){
	log_info(logger, "Generando metadata");
	printf("el directorio de la metadata es %s\n",DIR_metadata);
	char caracter_llenado;
	caracter_llenado=DIR_metadata[70];
	printf("el caracter de llenado es %c\n",caracter_llenado);

	FILE* metadata;
	metadata=fopen(DIR_metadata,"wt");

	t_config* temp=config_create(DIR_metadata);
	temp->path=DIR_metadata;
	char* md5;
	config_save_in_file(temp,DIR_metadata);
	if(caracter_llenado=='O'){
		md5=crear_MD5('O',0);
	config_set_value(temp,"SIZE","0");
	config_set_value(temp,"BLOCK_COUNT","0");
	config_set_value(temp,"BLOCKS","[]");
	config_set_value(temp,"CARACTER_LLENADO","O");//No me deja poner en "O" la variable caracter_llenado
	config_set_value(temp,"MD5_ARCHIVO",md5);
	config_save(temp);
	}
	else if (caracter_llenado=='C'){
		md5=crear_MD5('C',11);
		config_set_value(temp,"SIZE","0");
		config_set_value(temp,"BLOCK_COUNT","0");
		config_set_value(temp,"BLOCKS","[]");
		config_set_value(temp,"CARACTER_LLENADO","C");
		config_set_value(temp,"MD5_ARCHIVO",md5);
		config_save(temp);
	}
	else {
			md5=crear_MD5('B',27);
			config_set_value(temp,"SIZE","0");
			config_set_value(temp,"BLOCK_COUNT","0");
			config_set_value(temp,"BLOCKS","[]");
			config_set_value(temp,"CARACTER_LLENADO","B");
			config_set_value(temp,"MD5_ARCHIVO",md5);
			config_save(temp);
		}
	log_info(logger, "Archivo de metadata generado");
	fclose(metadata);

}
void crear_bitacora(char* DIR_bitacora){
	log_info(logger, "Generando Bitacora");
	FILE* bitacora;
	printf("el directorio de la bitacora es %s\n",DIR_bitacora);
	bitacora=fopen(DIR_bitacora,"w+");
	t_config* temp=config_create(DIR_bitacora);
	temp->path=DIR_bitacora;
	config_set_value(temp,"SIZE","0");
	config_set_value(temp,"BLOCKS","[]");

	config_save(temp);
	free(temp);
	fclose(bitacora);
	log_info(logger, "Se Genero la Bitacora");
	return;
}
char* obtener_directorio(char* nombre){
	t_config* config = config_create("imongostore.config");
	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
	char DIR_nombre[150];
	strcpy(DIR_nombre,punto_montaje);
	strcat(DIR_nombre,nombre);
	char *directorio=DIR_nombre;
	return directorio;
}
char* generar_directorio(char* nombre){

	char *directorio=obtener_directorio(nombre);
	mkdir(directorio,0755);
	printf("Se genero el directorio:  %s\n",directorio);
	return directorio;

}
void imprimir_bitmap(t_bitarray* bitmap){
	int cantidad_bits =blocks_amount;
//	printf("cant bits es %d\n",cantidad_bits);
	printf("Bitmap: ");
	for(int i=0;i<cantidad_bits;i++){
		if(bitarray_test_bit(bitmap, i) == 0){printf("0 ");}
		if(bitarray_test_bit(bitmap, i) == 1){printf("1 ");}
		//else {printf("Error de lectura\n");}
	}
	printf("\n");
}
/*
void* detector_sabotajes(void* s) {
	int socket_detector = esperar_cliente(*(int *)s);
	int posicion_x = 8, posicion_y = 8;

	log_info(logger, "Detector de sabotajes iniciado exitosamente");

	while(1) {
		sleep(10);

		t_mensaje* mensaje_out = crear_mensaje(SABO_P);

		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_x, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_y, ENTERO);

		enviar_mensaje(socket_detector, mensaje_out);

		liberar_mensaje(mensaje_out);
	}
}
void* rutina_trip(void* t) {
	tripulante* trip = (tripulante*) t;

	int socket_cliente = esperar_cliente(trip->socket_discord);
	log_info(logger, "Iniciado el tripulante %d de la patota %d", trip->id_trip, trip->id_patota);

	while(1) {
		t_mensaje* mensaje_out;
		t_list* mensaje_in = recibir_mensaje(socket_cliente);

		if(!validar_mensaje(mensaje_in, logger))
			log_info(logger, "Fallo el mensaje");

		switch ((int)list_get(mensaje_in, 0)) {
			case EXEC_1:
				log_info(logger, "EXEC_1 - Tripulante %d de la patota %d ejecutando tarea: %s", trip->id_trip, trip->id_patota, (char*)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case EXEC_0:
				log_info(logger, "EXEC_0 - Tripulante %d de la patota %d detuvo ejecucion", trip->id_trip, trip->id_patota);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_OX:
				log_info(logger, "GEN_OX - Tripulante %d de la patota %d generando %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_OX:
				log_info(logger, "CON_OX - Tripulante %d de la patota %d consumiendo %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_CO:
				log_info(logger, "GEN_CO - Tripulante %d de la patota %d generando %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_CO:
				log_info(logger, "CON_CO - Tripulante %d de la patota %d consumiendo %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_BA:
				log_info(logger, "GEN_BA - Tripulante %d de la patota %d generando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case DES_BA:
				log_info(logger, "DES_BA - Tripulante %d de la patota %d desechando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case ACTU_T:
				log_info(logger, "ACTU_T - Tripulante %d de la patota %d nueva posicion: %d|%d", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
		}
		liberar_mensaje(mensaje_out);
		list_destroy(mensaje_in);
	}
}


/*

	int server_fd = crear_conexion_servidor(IP_RAM, PUERTO_RAM);

		if(server_fd < 0) {
			log_info(logger, "Fallo en la conexión del servidor");
			close(server_fd);
			return ERROR_CONEXION;
		}

		log_info(logger, "Servidor listo para recibir al cliente");
		int socket_discord = esperar_cliente(server_fd);
		log_info(logger, "A llegado un nuevo cliente");


	//	t_list* lista;
	//	char* buffer;
		t_mensaje* respuesta;
		int largo_lista;

		while(1) {
			log_info(logger, "Entro a recibir mensaje");
					t_list* lista_parametros = recibir_mensaje(socket_discord);

					switch((int)list_get(lista_parametros, 0)) { // protocolo del mensaje
					case BITA_D:
						log_info(logger, "Largo de lista %d", lista_parametros->elements_count);
								largo_lista = lista_parametros->elements_count;
								log_info(logger, "op code: %d", (int )list_get(lista_parametros, 0));
								// memcpy(&op_code, (int *)list_get(lista_parametros, 0), sizeof(int));
								log_info(logger, "Recibi id_ tripulante %d", (int)list_get(lista_parametros, 1));
								log_info(logger, "Recibi id_ patota %d", (int)list_get(lista_parametros, 2));
								log_info(logger, "mensaje recibido, gorditos y bonitos muchachos");


								respuesta = crear_mensaje(TODOOK);
								enviar_mensaje(socket_discord, respuesta);
								return 0;
						break;

					case BITA_T:
						log_info(logger, "Largo de lista %d", lista_parametros->elements_count);
								largo_lista = lista_parametros->elements_count;
								log_info(logger, "op code: %d", (int )list_get(lista_parametros, 0));
								// memcpy(&op_code, (int *)list_get(lista_parametros, 0), sizeof(int));
								log_info(logger, "Recibi id_ tripulante %d", (int)list_get(lista_parametros, 1));
								log_info(logger, "Recibi id_ patota %d", (int)list_get(lista_parametros, 2));
								log_info(logger, "Recibi largo_mensaje %d", (int)list_get(lista_parametros, 3));
								log_info(logger, "mensaje %s", (char *)list_get(lista_parametros, 4));
								log_info(logger, "mensaje recibido, gorditos y bonitos muchachos");


								respuesta = crear_mensaje(TODOOK);
								enviar_mensaje(socket_discord, respuesta);
								return 0;
							break;

					case TAR_ES:
						log_info(logger, "Largo de lista %d", lista_parametros->elements_count);
								largo_lista = lista_parametros->elements_count;
								log_info(logger, "op code: %d", (int )list_get(lista_parametros, 0));
								// memcpy(&op_code, (int *)list_get(lista_parametros, 0), sizeof(int));
								log_info(logger, "tarea_code %s", (char *)list_get(lista_parametros, 1));
								log_info(logger, "parametro_tarea %d", (int)list_get(lista_parametros, 2));
								log_info(logger, "mensaje recibido, gorditos y bonitos muchachos");


								respuesta = crear_mensaje(TODOOK);
								enviar_mensaje(socket_discord, respuesta);
								return 0;
							break;


						break;
					default:
						log_warning(logger, "Operacion desconocida. No quieras meter la pata");
						return -1;
						break;
					}
				return EXIT_SUCCESS;
			}
 */

