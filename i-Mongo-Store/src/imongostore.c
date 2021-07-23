#include "imongostore.h"

uint32_t block_size;
uint32_t blocks_amount;
t_log* logger;
t_config* config;
t_bitarray *bitmap;

int tripu, pat = 0;
int main(void)
{

	logger=log_create("imongostore.log", "I-MONGO-STORE", 1, LOG_LEVEL_DEBUG);
	config=config_create("imongostore.config");
	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");

	log_info(logger, "toy aqui");
	//reviso si existe el archivo
	if(!access(punto_montaje,R_OK)){
		log_info(logger, "Ya esta creado el archivo");
		//que hago si existe?
	}
	else{
		log_info(logger, "Generando File System limpio");

		mkdir(punto_montaje,0755); //se crea el directorio /FileSystem/
		printf("el punto de montaje es %s\n",punto_montaje);

		char DIR_metadata[150];
		generar_directorio("/Files");
		generar_directorio("/Files/Bitacoras");
		strcpy(DIR_metadata,obtener_directorio("/Files/Oxigeno.ims"));
		crear_metadata(DIR_metadata);

		strcpy(DIR_metadata,obtener_directorio("/Files/Comida.ims"));
		crear_metadata(DIR_metadata);

		strcpy(DIR_metadata,obtener_directorio("/Files/Basura.ims"));
		crear_metadata(DIR_metadata);

		//creo el superBloque
		char DIR_superBloque[100];
		strcpy(DIR_superBloque,obtener_directorio("/superBloque.ims"));
		printf("el directorio del superbloque es %s\n",DIR_superBloque);
		block_size=config_get_long_value(config, "BLOCK_SIZE");
		blocks_amount=config_get_long_value(config, "BLOCKS_AMOUNT");
		crear_superBloque(DIR_superBloque);

 //prueba de lectura del super bloque
/*
		FILE *fp=fopen(DIR_superBloque,"r+");

			fread(&prueba2,sizeof(uint32_t),1,fp);
			printf("El tamanio de bloque es %u\n",prueba2);
			fread(&prueba2,sizeof(uint32_t),1,fp);
			printf("La cantidad de bloques es %u\n",prueba2);
			fread(&bitmap,sizeof(bitmap_size),1,fp);

*/
		imprimir_bitmap(bitmap);


	log_info(logger, "Creando conexiones");

	}
}

/*
char* crear_superBloque(char* DIR_superBloque){//ver como hacer para pasar uint32_t* como parametro sin que rompa
	printf("el tamaño del bloque es %d y la cant de bloques es %d \n",block_size,blocks_amount);

	int bitmap_size=(blocks_amount/8);
	int superBloque_size=(bitmap_size+ 2*sizeof(uint32_t));
	printf("el tamaño del super bloque es %d\n",superBloque_size);
	//Crear bitmap?
	printf("el directorio del superBloque es %s\n",DIR_superBloque);
	FILE* fp;
	fp = fopen(DIR_superBloque,"w+");
	if(fp==NULL){
		printf("no se pudo abrir/generar el archivo\n");
		return(-1);
	}

	t_config* temp=config_create(DIR_superBloque);
	temp->path=DIR_superBloque;
	config_save_in_file(temp,DIR_superBloque);
	config_set_value(temp,"BLOCK_SIZE",""); //no puedo ingresar el valor de block_size
	config_set_value(temp,"BLOCKS","");
	config_set_value(temp,"BITMAP","");
	config_save(temp);

	fclose(fp);
	char* puntero_SuperBloque=DIR_superBloque;;
	printf("se creo el super bloque\n");

	return puntero_SuperBloque;
}*/
/*
char* crear_superBloque(char* DIR_superBloque){//ver como hacer para pasar uint32_t* como parametro sin que rompa
	printf("el tamaño del bloque es %d y la cant de bloques es %d \n",block_size,blocks_amount);
	uint32_t data[2]={block_size,blocks_amount};
	int bitmap_size=(blocks_amount/8);

	//Crear bitmap?
	printf("el directorio del superBloque es %s\n",DIR_superBloque);
	FILE* fp;
	fp = fopen(DIR_superBloque,"w+");
	if(fp==NULL){
		printf("no se pudo abrir/generar el archivo\n");
		return(-1);
	}
	ftruncate(fp,sizeof(uint32_t)*2+bitmap_size);
	fwrite(data,sizeof(uint32_t),2,fp);
	printf("se escribio el archivo\n");
//genero todo en 0 el bitmap
	char* temp_bitmap= malloc(bitmap_size);
	t_bitarray *bitmap=bitarray_create(temp_bitmap,bitmap_size);
	for(int i=0;i<bitarray_get_max_bit(bitmap);i++){
		bitarray_clean_bit(bitmap,i);
	}

	fwrite(bitmap,bitmap_size,1,fp);
	printf("se escribio el archivo\n");
	fclose(fp);
	char* puntero_SuperBloque=DIR_superBloque;;
	printf("se creo el super bloque\n");

	return puntero_SuperBloque;
}*/
char* crear_superBloque(char* DIR_superBloque){//ver como hacer para pasar uint32_t* como parametro sin que rompa
	printf("el tamaño del bloque es %d y la cant de bloques es %d \n",block_size,blocks_amount);
	int bitmap_size=(blocks_amount/8);

	//Crear bitmap?
	printf("el directorio del superBloque es %s\n",DIR_superBloque);
	int fp = open(DIR_superBloque, O_CREAT | O_RDWR, 0664);

	if(fp==-1){
		printf("no se pudo abrir/generar el archivo\n");
		return(-1);
	}
	printf("el directorio del superBloque es \n");
	ftruncate(fp,sizeof(uint32_t)*2+bitmap_size);
	void* superBloque = mmap(NULL, sizeof(uint32_t)*2 + bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
//genero todo en 0 el bitmap
	if (superBloque == MAP_FAILED) {
				log_error(logger, "Error de mmap");
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
	printf("se creo el super bloque\n");

	return puntero_SuperBloque;
}
void* crear_blocks(char* DIR_blocks, int tamanio_blocks, int blocks_file){

        int blocks_file_copy;

	blocks_file = open(DIR_blocks, O_CREAT | O_RDWR, 0666);
	if (blocks_file == -1){

		printf("No se puede abrir el archivo %s\n", DIR_blocks);
		exit(-1);
	}
	printf("Generando Blocks.ims...\n");
	ftruncate(blocks_file,tamanio_blocks);
	void* blocks_mem = mmap(NULL, tamanio_blocks, PROT_READ | PROT_WRITE,MAP_SHARED, blocks_file, 0);
	if (blocks_mem == MAP_FAILED){
		printf("Error al mapear superbloque\n");
		exit(-1);
	}
	return(blocks_mem);

        blocks_file_copy = open(DIR_blocks,O_CREAT|O_RDWR,066),
        printf("Creando copia dEL BLOCK\n");

        ftruncate(blocks_file_copy,tamanio_blocks);

       void* blocks_mem_copy;
       blocks_mem_copy= mmap(NULL,tamanio_blocks,PROT_READ|PROT_WRITE,MAP_SHARED,blocks_file_copy,0);
       memcpy(blocks_file_copy,&blocks_file,tamanio_blocks);
       close(blocks_file_copy);
       close(blocks_file);

       return(blocks_file_copy);

}
void crear_metadata(char* DIR_metadata){

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
	config_set_value(temp,"SIZE","123");
	config_set_value(temp,"BLOCK_COUNT","4");
	config_set_value(temp,"BLOCKS","[5,6,7,8]");
	config_set_value(temp,"CARACTER_LLENADO","O");//No me deja poner en "O" la variable caracter_llenado
	config_set_value(temp,"MD5_ARCHIVO",md5);
	config_save(temp);
	}
	else if (caracter_llenado=='C'){
		md5=crear_MD5('C',11);
		config_set_value(temp,"SIZE","123");
		config_set_value(temp,"BLOCK_COUNT","4");
		config_set_value(temp,"BLOCKS","[1,2,3,4]");
		config_set_value(temp,"CARACTER_LLENADO","C");
		config_set_value(temp,"MD5_ARCHIVO",md5);
		config_save(temp);
	}
	else {
			md5=crear_MD5('B',27);
			config_set_value(temp,"SIZE","123");
			config_set_value(temp,"BLOCK_COUNT","4");
			config_set_value(temp,"BLOCKS","[9,10,11,12]");
			config_set_value(temp,"CARACTER_LLENADO","B");
			config_set_value(temp,"MD5_ARCHIVO",md5);
			config_save(temp);
		}
	fclose(metadata);

}
void generar_bitacora(char* DIR_bitacora){
	FILE* bitacora;
	printf("el directorio de la bitacora es %s\n",DIR_bitacora);
	bitacora=fopen(DIR_bitacora,"w+");
	t_config* temp=config_create(DIR_bitacora);
	temp->path=DIR_bitacora;
	config_set_value(temp,"SIZE","");
	config_set_value(temp,"BLOCKS","");

	config_save(temp);
	config_destroy(temp);
	fclose(bitacora);
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
	int cantidad_bits =16; //bitarray_get_max_bit(bitmap); //arreglar
	printf("cant bits es %d\n",cantidad_bits);
	for(int i=0;i<cantidad_bits;i++){
		if(bitarray_test_bit(bitmap, i) == 0){printf("Posicion: %i = 0\n", i);}
		if(bitarray_test_bit(bitmap, i) == 1){printf("Posicion: %i = 1\n", i);}
		//else {printf("Error de lectura\n");}
	}
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

