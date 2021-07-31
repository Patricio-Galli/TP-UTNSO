#include "mongo.h"

t_config* config;
t_log* logger;

int tripu, pat = 0;

int main() {

	logger = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_INFO);
    config = config_create("imongostore.config");
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
/*
    		strcpy(DIR_metadata,obtener_directorio("/Files/Oxigeno.ims"));
    		crear_metadata(DIR_metadata);

    		strcpy(DIR_metadata,obtener_directorio("/Files/Comida.ims"));
    		crear_metadata(DIR_metadata);

    		strcpy(DIR_metadata,obtener_directorio("/Files/Basura.ims"));
    		crear_metadata(DIR_metadata);
*/
    		int id_tripulante=1;
    		strcpy(DIR_metadata,obtener_directorio("/Files/Bitacoras/Tripulante"));
    		strcat(DIR_metadata,string_itoa(id_tripulante));
    		strcat(DIR_metadata,".ims");
    		crear_bitacora(DIR_metadata);

    		imprimir_bitmap(bitmap);
    		printf("sumar 10 oxigenos \n");
    		sumar_caracteres('O',10);
    		imprimir_bitmap(bitmap);
    		printf("consumir 12 oxigenos \n");
    		quitar_caracteres('O',12);
    		imprimir_bitmap(bitmap);

    		printf("sumar 24 comidas \n");
    		sumar_caracteres('C',24);
    		imprimir_bitmap(bitmap);
    		printf("consumir 12 Comidas \n");
    		quitar_caracteres('C',12);
    		imprimir_bitmap(bitmap);

    		printf("sumar 10 basuras \n");
    		    		sumar_caracteres('B',10);
    		    		imprimir_bitmap(bitmap);
    		    		printf("consumir 1 Basura \n");
    		    		quitar_caracteres('B',1);
    		    		imprimir_bitmap(bitmap);

    		tripulante* temp2=malloc(sizeof(tripulante));
    		temp2->id_trip=1;temp2->posicion_x=1;temp2->posicion_y=2;temp2->id_patota=1;temp2->socket_discord=2;
    		actualizar_posicion(temp2,3,4,"/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/Files/Bitacoras/Tripulante1.ims");
    		imprimir_bitmap(bitmap);

    		comienza_tarea("Sacar basura","/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/Files/Bitacoras/Tripulante1.ims");
    		imprimir_bitmap(bitmap);

    		finaliza_tarea("Sacar basura","/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/Files/Bitacoras/Tripulante1.ims");
    		imprimir_bitmap(bitmap);

    		inicio_sabotaje("/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/Files/Bitacoras/Tripulante1.ims");
    		imprimir_bitmap(bitmap);

    		fin_sabotaje("/home/utnso/tp-2021-1c-cualquier-cosa/i-Mongo-Store/Filesystem2/Files/Bitacoras/Tripulante1.ims");
    		imprimir_bitmap(bitmap);

    	log_info(logger, "Creando conexiones");


    //------------------------------------comunicaciones-------------------------------------
	int server_fd = crear_conexion_servidor(IP_MONGO, config_get_int_value(config, "PUERTO"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	log_info(logger, "Conexión establecida con el discordiador");

	while(1) {
		log_info(logger, "Esperando información del discordiador");

		t_list* mensaje_in = recibir_mensaje(socket_discord);
		t_mensaje* mensaje_out;

		if (!validar_mensaje(mensaje_in, logger)) {
			log_info(logger, "Cliente desconectado dentro del while");
			close(server_fd);
			log_destroy(logger);
			return ERROR_CONEXION;
		}

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
			case INIT_S:
				log_info(logger, "Iniciando el detector de sabotajes");
				int socket_detector = crear_conexion_servidor(IP_MONGO, 0, 1);

				pthread_t hilo_detector_sabotajes;
				pthread_create(&hilo_detector_sabotajes, NULL, detector_sabotajes, &socket_detector);

				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(socket_detector), ENTERO);
				enviar_mensaje(socket_discord, mensaje_out);

				break;
			case INIT_P:
				log_info(logger, "Discordiador inicio una patota");

				pat++;
				tripu = 1;

				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_discord, mensaje_out);
				break;
			case INIT_T:
				log_info(logger, "Discordiador solicito iniciar un tripulante");
				tripulante* trip = malloc(sizeof(tripulante));

				trip->id_patota = pat;
				trip->id_trip = tripu;
				trip->posicion_x = (int)list_get(mensaje_in, 1);
				trip->posicion_y = (int)list_get(mensaje_in, 2);
				trip->socket_discord = crear_conexion_servidor(IP_MONGO, 0, 1);

				pthread_t hilo_nuevo;
				pthread_create(&hilo_nuevo, NULL, rutina_trip, trip);

				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(trip->socket_discord), ENTERO);
				enviar_mensaje(socket_discord, mensaje_out);

				tripu++;
				break;
			case BITA_D:
				log_info(logger, "Discordiador solicito la bitacora del tripulante %d de la patota %d", (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));
				int cantidad_lineas_archivo = 4;

				mensaje_out = crear_mensaje(BITA_C);

				agregar_parametro_a_mensaje(mensaje_out, (void*)cantidad_lineas_archivo, ENTERO);
				for(int i = 1; i <= cantidad_lineas_archivo; i++) {
					char linea[25];
					sprintf(linea, "%d) BITACORA ..........", i);

					agregar_parametro_a_mensaje(mensaje_out, (void*)linea, BUFFER); //esto es lo unico que tienen que mantener adentro del for, despues tienen que ir cargando en linea las lineas del archivo de bitacora
				}

				enviar_mensaje(socket_discord, mensaje_out);
				break;
			default:
				log_warning(logger, "No entendi el mensaje");
				break;
		}
		list_destroy(mensaje_in);
		liberar_mensaje(mensaje_out);
	}

	log_warning(logger, "FINALIZANDO MONGO");
	return 0;
}

void* detector_sabotajes(void* s) {
	int socket_detector = esperar_cliente(*(int *)s);
	int posicion_x = 8, posicion_y = 8;

	log_info(logger, "Detector de sabotajes iniciado exitosamente");

	while(1) {
		sleep(10);

		log_warning(logger, "ENVIANDO SABOTAJE AL DISCORDIDADOR");
		t_mensaje* mensaje_out = crear_mensaje(SABO_P);

		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_x, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_y, ENTERO);

		enviar_mensaje(socket_detector, mensaje_out);
		liberar_mensaje(mensaje_out);

		t_list* mensaje_in = recibir_mensaje(socket_detector);

		mensaje_out = crear_mensaje(TODOOK);

		if((int)list_get(mensaje_in, 0) == SABO_I) {
			log_info(logger, "Tripulante empezo a resolver el sabotaje");
			enviar_mensaje(socket_detector, mensaje_out);
			list_destroy(mensaje_in);

			mensaje_in = recibir_mensaje(socket_detector);

			if((int)list_get(mensaje_in, 0) == SABO_F) {
				log_info(logger, "Tripulante termino de resolver el sabotaje");
				enviar_mensaje(socket_detector, mensaje_out);
			}

			list_destroy(mensaje_in);
			liberar_mensaje(mensaje_out);
		}else
			log_error(logger, "No se pudo resolver el sabotaje");
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
			int cantidad_caracteres=(int)list_get(mensaje_in, 0);
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
				sumar_caracteres('O',cantidad_caracteres);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_OX:
				log_info(logger, "CON_OX - Tripulante %d de la patota %d consumiendo %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				quitar_caracteres('O',cantidad_caracteres);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_CO:
				log_info(logger, "GEN_CO - Tripulante %d de la patota %d generando %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				sumar_caracteres('C',cantidad_caracteres);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_CO:
				log_info(logger, "CON_CO - Tripulante %d de la patota %d consumiendo %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				quitar_caracteres('C',cantidad_caracteres);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_BA:
				log_info(logger, "GEN_BA - Tripulante %d de la patota %d generando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				sumar_caracteres('B',cantidad_caracteres);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case DES_BA:
				log_info(logger, "DES_BA - Tripulante %d de la patota %d desechando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				quitar_caracteres('B',cantidad_caracteres);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case ACTU_T:
				log_info(logger, "ACTU_T - Tripulante %d de la patota %d nueva posicion: %d|%d", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));
				//actualizar_posicion(trip,(int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), DIR_Bit_Tripulante)//calcular DIR_Bit_tripulante
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
		}
		liberar_mensaje(mensaje_out);
		list_destroy(mensaje_in);
	}
}
char* crear_superBloque(char* DIR_superBloque){
	log_info(logger, "Verificando existencia SuperBloque");
	FILE* existe= fopen(DIR_superBloque,"r");
	int bitmap_size=roundUp(blocks_amount,8);
	if(existe != NULL){
		log_info(logger, "SuperBloque ya existe");
	/*	fseek(existe,sizeof(uint32_t)*2+bitmap_size,SEEK_SET);
		printf("se aplico FSEEK \n");
		fread(bitmap,bitmap_size,1,existe);
		printf("se guardo el bitmap\n");
		imprimir_bitmap(bitmap);
		//msync(bitmap->bitarray,bitmap_size,MS_SYNC);
		//msync(bitmap, DIR_superBloque+sizeof(uint32_t)*2 + bitmap_size, MS_SYNC);
		//memcpy(bitmap,DIR_superBloque,sizeof(uint32_t)*2);
		fclose(existe);
		return DIR_superBloque;*/
	}
	else
		log_info(logger, "SuperBloque no existe, creandolo.");
	printf("el tamaño del bloque es %d y la cant de bloques es %d \n",block_size,blocks_amount);


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
/*
	for(int i=0;i<bitarray_get_max_bit(bitmap);i++){
		bitarray_clean_bit(bitmap,i);
	}*/
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
	char* puntero_SuperBloque=DIR_superBloque;
	log_info(logger, "SuperBloque Generado");

	return puntero_SuperBloque;
}
char* crear_blocks(char* DIR_blocks){
	log_info(logger, "Verificando existencia Blocks");
		FILE* existe= fopen(DIR_blocks,"r");
		if(existe != NULL){
			log_info(logger, "Blocks ya existe");
			int size=block_size*blocks_amount;
			blocks_copy= malloc(size);
			fclose(existe);
			return DIR_blocks;
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
	log_info(logger, "Buscando archivos ya existentes");
	printf("el directorio de la metadata es %s\n",DIR_metadata);
	char caracter_llenado;
	caracter_llenado=DIR_metadata[70];
	printf("el caracter de llenado es %c\n",caracter_llenado);

	FILE* metadata;
	metadata=fopen(DIR_metadata,"rb");

	if(metadata!=NULL){
		log_info(logger, "Archivo existente encontrado");
		fclose(metadata);
		return;
	}
	log_info(logger, "Archivos previos no encontrados, Generando metadata");
	metadata=fopen(DIR_metadata,"wb");
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
		md5=crear_MD5('C',0);
		config_set_value(temp,"SIZE","0");
		config_set_value(temp,"BLOCK_COUNT","0");
		config_set_value(temp,"BLOCKS","[]");
		config_set_value(temp,"CARACTER_LLENADO","C");
		config_set_value(temp,"MD5_ARCHIVO",md5);
		config_save(temp);
	}
	else {
			md5=crear_MD5('B',0);
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
	log_info(logger, "Buscando archivos ya existentes");
	FILE* metadata;
		metadata=fopen(DIR_bitacora,"rb");

		if(metadata!=NULL){
			log_info(logger, "Archivo existente encontrado");
			fclose(metadata);
			return;
		}
		else
			log_info(logger, "Archivos previos no encontrados, Generando bitacora");
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


