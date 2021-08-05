#include "mongo.h"

t_config* config;
t_log* logger;

int id_trip_global, id_patota_global = 0;

int main() {
	logger = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_INFO);
	config = config_create("imongostore.config");
	punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");

	int server_fd = crear_conexion_servidor(IP_MONGO, config_get_int_value(config, "PUERTO"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		return ERROR_CONEXION;
	}

	log_info(logger, "Servidor listo");
	int socket_discord = esperar_cliente(server_fd);
	log_info(logger, "Conexión establecida con el discordiador");

	mkdir(punto_montaje,0755); //se crea el directorio /FileSystem/

	char DIR_superBloque[100];
	strcpy(DIR_superBloque,obtener_directorio("/superBloque.ims")); //1

	block_size = config_get_long_value(config, "BLOCK_SIZE");
	blocks_amount = config_get_long_value(config, "BLOCKS_AMOUNT");

	crear_superBloque(DIR_superBloque);//creo el superBloque (si no esta creado)
	free(DIR_superBloque);

	char DIR_Blocks[100];
	strcpy(DIR_Blocks,obtener_directorio("/Blocks.ims"));
	crear_blocks(DIR_Blocks);
	free(DIR_Blocks);

	pthread_create(&hilo_actualizador_block,NULL,uso_blocks,&blocks);

	generar_directorio("/Files");
	generar_directorio("/Files/Bitacoras");

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

				id_patota_global++;
				id_trip_global = 1;

				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_discord, mensaje_out);
				break;
			case INIT_T:
				log_info(logger, "Discordiador solicito iniciar un tripulante");
				tripulante* trip = malloc(sizeof(tripulante));

				trip->id_patota = id_patota_global;
				trip->id_trip = id_trip_global;
				trip->posicion_x = (int)list_get(mensaje_in, 1);
				trip->posicion_y = (int)list_get(mensaje_in, 2);
				trip->socket_discord = crear_conexion_servidor(IP_MONGO, 0, 1);

				char DIR_metadata[150];

				strcpy(DIR_metadata,obtener_directorio("/Files/Bitacoras/Tripulante"));
				strcat(DIR_metadata,string_itoa(trip->id_trip));
				strcat(DIR_metadata,".ims");
				crear_bitacora(DIR_metadata);

				pthread_t hilo_nuevo;
				pthread_create(&hilo_nuevo, NULL, rutina_trip, trip);

				mensaje_out = crear_mensaje(SND_PO);
				agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(trip->socket_discord), ENTERO);
				enviar_mensaje(socket_discord, mensaje_out);

				id_trip_global++;
				break;
			case BITA_D:

				log_info(logger, "Discordiador solicito la bitacora del tripulante %d de la patota %d", (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));


				mensaje_out = crear_mensaje(BITA_C);
				char* string_bitacora = string_new();
				string_bitacora = obtener_bitacora((int)list_get(mensaje_in, 1));
				char**lineas_bitacora = string_split(string_bitacora,".");
				int cantidad_lineas = 0;

				while(lineas_bitacora[cantidad_lineas+1] != NULL)
				    cantidad_lineas++;

				agregar_parametro_a_mensaje(mensaje_out, (void*)cantidad_lineas, ENTERO);

				for(int i = 1; 0 <= cantidad_lineas; i++)
					agregar_parametro_a_mensaje(mensaje_out, (void*)lineas_bitacora[cantidad_lineas], BUFFER);

				enviar_mensaje(socket_discord, mensaje_out);
				break;
			default:
				log_warning(logger, "No entendi el mensaje");
				break;
		}
		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}

	log_warning(logger, "FINALIZANDO MONGO");
	free(blocks);
	return 0;
}

void* detector_sabotajes(void* s) {
	int socket_detector = esperar_cliente(*(int *)s);
	int posicion_x = 8, posicion_y = 8;

	log_info(logger, "Detector de sabotajes iniciado exitosamente");

	while(1) {
		sleep(9999);

		log_warning(logger, "ENVIANDO SABOTAJE AL DISCORDIDADOR");
		t_mensaje* mensaje_out = crear_mensaje(SABO_P);

		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_x, ENTERO);
		agregar_parametro_a_mensaje(mensaje_out, (void*)posicion_y, ENTERO);

		enviar_mensaje(socket_detector, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list* mensaje_in = recibir_mensaje(socket_detector);

		mensaje_out = crear_mensaje(TODOOK);

		if((int)list_get(mensaje_in, 0) == SABO_I) {
			log_info(logger, "Tripulante empezo a resolver el sabotaje");
			enviar_mensaje(socket_detector, mensaje_out);
			liberar_mensaje_in(mensaje_in);

			mensaje_in = recibir_mensaje(socket_detector);

			if((int)list_get(mensaje_in, 0) == SABO_F) {
				log_info(logger, "Tripulante termino de resolver el sabotaje");
				enviar_mensaje(socket_detector, mensaje_out);
			}

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
		}else
			log_error(logger, "No se pudo resolver el sabotaje");

		sleep(600);
	}
}

void* rutina_trip(void* t) {
	tripulante* trip = (tripulante*) t;

	int socket_cliente = esperar_cliente(trip->socket_discord);
	log_info(logger, "Iniciado el tripulante %d de la patota %d", trip->id_trip, trip->id_patota);

	char*DIR_Bit_Tripulante=string_new();

	string_append(&DIR_Bit_Tripulante,obtener_directorio("/Files/Bitacoras/Tripulante"));
	string_append(&DIR_Bit_Tripulante,string_itoa(trip->id_trip));
	string_append(&DIR_Bit_Tripulante,".ims");

	while(1) {
		t_mensaje* mensaje_out;
		t_list* mensaje_in = recibir_mensaje(socket_cliente);

		if(!validar_mensaje(mensaje_in, logger))
		log_info(logger, "Fallo el mensaje");

		switch ((int)list_get(mensaje_in, 0)) {
			case EXEC_1:
				log_info(logger, "EXEC_1 - Tripulante %d de la patota %d ejecutando tarea: %s", trip->id_trip, trip->id_patota, (char*)list_get(mensaje_in, 1));
				comienza_tarea((char*)list_get(mensaje_in, 1),DIR_Bit_Tripulante);
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
				//comienza_tarea("Generando Oxigeno",DIR_Bit_Tripulante);
				sumar_caracteres('O',(int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_OX:
				log_info(logger, "CON_OX - Tripulante %d de la patota %d consumiendo %d de oxigeno", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				//comienza_tarea("Consumiendo Oxigeno",DIR_Bit_Tripulante);
				quitar_caracteres('O',(int)list_get(mensaje_in, 1));
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_CO:
				log_info(logger, "GEN_CO - Tripulante %d de la patota %d generando %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				sumar_caracteres('C',(int)list_get(mensaje_in, 1));
				//comienza_tarea("Generando Comida",DIR_Bit_Tripulante);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case CON_CO:
				log_info(logger, "CON_CO - Tripulante %d de la patota %d consumiendo %d de comida", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				quitar_caracteres('C',(int)list_get(mensaje_in, 1));
				//comienza_tarea("Consumiendo Comida",DIR_Bit_Tripulante);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case GEN_BA:
				log_info(logger, "GEN_BA - Tripulante %d de la patota %d generando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				sumar_caracteres('B',(int)list_get(mensaje_in, 1));
				comienza_tarea("Generando Basura",DIR_Bit_Tripulante);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case DES_BA:
				log_info(logger, "DES_BA - Tripulante %d de la patota %d desechando %d de basura", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1));
				quitar_caracteres('B',(int)list_get(mensaje_in, 1));
				//comienza_tarea("Desechando Basura",DIR_Bit_Tripulante);
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
			case ACTU_P:
				log_info(logger, "ACTU_P - Tripulante %d de la patota %d nueva posicion: %d|%d", trip->id_trip, trip->id_patota, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2));
				actualizar_posicion(trip,(int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), DIR_Bit_Tripulante);//calcular DIR_Bit_tripulante
				mensaje_out = crear_mensaje(TODOOK);
				enviar_mensaje(socket_cliente, mensaje_out);
				break;
		}
		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(mensaje_in);
		//free(DIR_Bit_Tripulante);
	}
}
void crear_superBloque(char* DIR_superBloque){
	log_info(logger, "Verificando existencia SuperBloque");

	FILE* existe = fopen(DIR_superBloque,"r");

	int bitmap_size=roundUp(blocks_amount,8);

	if(existe != NULL)
		log_info(logger, "SuperBloque ya existe");
	else
		log_info(logger, "SuperBloque no existe, creandolo.");

	log_info(logger, "el tamaño del bloque es %d y la cant de bloques es %d \n",block_size,blocks_amount);
	log_info(logger, "el directorio del superBloque es %s\n",DIR_superBloque);

	int fp = open(DIR_superBloque, O_CREAT | O_RDWR, 0664);

	if(fp==-1){
		log_info(logger, "No se pudo abrir/generar el archivo");
		exit(-1);
	}

	ftruncate(fp,sizeof(uint32_t)*2+bitmap_size);

	void* superBloque = mmap(NULL, sizeof(uint32_t)*2 + bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);

	if (superBloque == MAP_FAILED) {
		log_error(logger, "Error al mapear el SuperBloque");
		close(fp);
	} else {
		bitmap = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), bitmap_size, MSB_FIRST);

		void* prueba = malloc(4);

		memcpy(prueba,&block_size,sizeof(uint32_t));
		memcpy(superBloque,prueba,sizeof(uint32_t));
		memcpy(prueba,&blocks_amount,sizeof(uint32_t));
		memcpy(superBloque+sizeof(uint32_t),prueba,sizeof(uint32_t));

		msync(bitmap->bitarray,bitmap_size,MS_SYNC);
		msync(superBloque, sizeof(uint32_t)*2 + bitmap_size, MS_SYNC);

		log_info(logger, "se escribio el archivo\n");

		close(fp);
		free(prueba);

		log_info(logger, "SuperBloque Generado");
	}
}

void crear_blocks(char* DIR_blocks){
	log_info(logger, "Verificando existencia Blocks");
	int size = block_size * blocks_amount;

	FILE* existe= fopen(DIR_blocks,"r");

	if(existe != NULL){
		log_info(logger, "Blocks ya existe");

		blocks_copy= malloc(size);

		fclose(existe);
	} else {
		log_info(logger, "Blocks no existe, creandolo");

		int fp = open(DIR_blocks, O_CREAT | O_RDWR, 0666);

		if (fp == -1){
			log_info(logger, "No se pudo abrir/generar el archivo");
			exit(-1);
		}

		ftruncate(fp, size);

		blocks = mmap(NULL, size, PROT_READ | PROT_WRITE,MAP_SHARED, fp, 0);

		if(blocks == MAP_FAILED){
			log_info(logger, "Error al mapear Blocks");
			exit(-1);
		} else {
			log_info(logger, "Blocks Generado");
			//blocks_copy= malloc(size); todo verificar
			close(fp);
		}
	}
}
void* uso_blocks(){//se deberia encargar un hilo de esto?
	int size=block_size*blocks_amount;
	blocks_copy= malloc(size);
	while(1){

	sleep(config_get_int_value(config, "TIEMPO_SINCRONIZACION"));
	pthread_mutex_lock(&actualizar_blocks);
		memcpy(blocks_copy,blocks,size);
	pthread_mutex_unlock(&actualizar_blocks);
	//log_info(logger, "Se realizo copia de blocks");
	msync(blocks,(size), MS_SYNC);
	//log_info(logger, "Se sincronizo el blocks");
	}
	free(blocks_copy);
}
void crear_metadata(char* DIR_metadata, char caracter_llenado){
	log_info(logger, "Buscando archivos de recursos ya existentes");

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
	config_destroy(temp);
	fclose(bitacora);
	log_info(logger, "Se Genero la Bitacora");
	return;
}
char* obtener_directorio(char* nombre) {
	char* DIR_nombre = string_new();

	string_append(&DIR_nombre,punto_montaje);
	string_append(&DIR_nombre,nombre);

	return DIR_nombre;
}

void generar_directorio(char* nombre) {
	char *directorio = obtener_directorio(nombre);

	mkdir(directorio,0755);
	log_info(logger, "Se genero el directorio:  %s",directorio);
	free(directorio);
}

void imprimir_bitmap(t_bitarray* bitmap) {
	char* bits = string_new();

	for(int i = 0; i<blocks_amount ;i++){
		string_append(&bits,bitarray_test_bit(bitmap, i));
		string_append(&bits," ");
	}

	log_info(logger, "Bitmap: %s", bits);
	free(bits);
}
char* obtener_bitacora(int trip_id){

	char*DIR_Bit_Tripulante=string_new();
	string_append(&DIR_Bit_Tripulante,obtener_directorio("/Files/Bitacoras/Tripulante"));
	string_append(&DIR_Bit_Tripulante,string_itoa(trip_id));
	string_append(&DIR_Bit_Tripulante,".ims");
	t_config* bitacora_meta=config_create(DIR_Bit_Tripulante);
	int size=config_get_int_value(bitacora_meta,"SIZE");
	char**bloques=config_get_array_value(bitacora_meta,"BLOCKS");

	char*temp=malloc(block_size);
	int desplazamiento;
	char* string_bitacora=string_new();
	for(int i=0;i<roundUp(size,block_size)-1;i++){
		desplazamiento=(atoi(bloques[i]))*block_size;
		memcpy(temp,blocks+desplazamiento,block_size);// cambiar por blocks original
		temp[block_size]='\0';
		string_append(&string_bitacora,temp);
		if(i+2==roundUp(size,block_size)){
			desplazamiento=(atoi(bloques[i+1]))*block_size;
			memcpy(temp,blocks_copy+desplazamiento,size%block_size);//revisar, si el ultimo bloque no esta completo, me esccribe cualquier cosa
			temp[size%block_size]='\0';
			string_append(&string_bitacora,temp);
		}
	}
	printf("final: %s\n",string_bitacora);
	liberar_input(bloques);
	free(DIR_Bit_Tripulante);
	config_destroy(bitacora_meta);
	return string_bitacora;
}

void liberar_input(char** input) {
	int i = 0;

	while(input[i] != NULL) {
		free(input[i]);
		i++;
	}

	free(input);
}

