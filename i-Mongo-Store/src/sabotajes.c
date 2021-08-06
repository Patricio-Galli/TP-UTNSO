#include "sabotajes.h"

void inicializar_detector_sabotajes(int socket_discord) {
	ubicaciones_sabotajes = config_get_array_value(config, "POSICIONES_SABOTAJE");
	contador_sabotajes = 0;

	signal(SIGUSR1, analizador_sabotajes);

	int socket_d = crear_conexion_servidor(IP_MONGO, 0, 1);

	t_mensaje* mensaje_out = crear_mensaje(SND_PO);
	agregar_parametro_a_mensaje(mensaje_out, (void *)puerto_desde_socket(socket_d), ENTERO);
	enviar_mensaje(socket_discord, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	socket_sabotajes = esperar_cliente(socket_d);
	log_info(logger, "Detector de sabotajes iniciado exitosamente");
}

void analizador_sabotajes(int senial) {
	char** ubicacion_dividida = string_split(ubicaciones_sabotajes[contador_sabotajes], "|");
	int pos_x = atoi(ubicacion_dividida[0]);
	int pos_y = atoi(ubicacion_dividida[1]);
	liberar_split(ubicacion_dividida);

	log_info(logger, "Sabotaje recibido en ubicacion %d|%d", pos_x, pos_y);

	log_warning(logger, "Enviando sabotaje al discordiador");
	t_mensaje* mensaje_out = crear_mensaje(SABO_P);
	agregar_parametro_a_mensaje(mensaje_out, (void*)pos_x, ENTERO);
	agregar_parametro_a_mensaje(mensaje_out, (void*)pos_y, ENTERO);
	enviar_mensaje(socket_sabotajes, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list* mensaje_in = recibir_mensaje(socket_sabotajes);

	mensaje_out = crear_mensaje(TODOOK);

	if((int)list_get(mensaje_in, 0) == SABO_I) {
		int id_trip = (int)list_get(mensaje_in, 1);
		int id_patota = (int)list_get(mensaje_in, 2);
		tripulante* trip = obtener_tripulante(id_trip, id_patota);

		log_info(logger, "El tripulante %d de la patota %d esta yendo a la ubicacion del sabotaje", id_trip, id_patota);
		inicio_sabotaje(trip->dir_bitacora);

		enviar_mensaje(socket_sabotajes, mensaje_out);
		liberar_mensaje_in(mensaje_in);

		mensaje_in = recibir_mensaje(socket_sabotajes);

		if((int)list_get(mensaje_in, 0) == SABO_F) {
			log_info(logger, "El tripulante llego a la ubicacion del sabotaje");
			fin_sabotaje(trip->dir_bitacora);
			enviar_mensaje(socket_sabotajes, mensaje_out);
			//todo RESOLVER SABOTAJE
			contador_sabotajes++;
		} else
			log_error(logger, "Se Murio");

		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}else {
		log_error(logger, "No se pudo resolver el sabotaje");
		liberar_mensaje_in(mensaje_in);
		liberar_mensaje_out(mensaje_out);
	}
}

//////////////////////////////////FUNCIONES RESOLVEDORAS////////////////////////////////

void arreglar_BlockCount_superBloque() {
	char* DIR_SuperBLoque=string_new();
	string_append(&DIR_SuperBLoque,obtener_directorio("/superBloque.ims"));
	int superBlock_file = open(DIR_SuperBLoque, O_CREAT | O_RDWR, 0664);

	void* superBloque = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, superBlock_file, 0);

	uint32_t SUP_block_size;
	memcpy(&SUP_block_size, superBloque, sizeof(uint32_t));

	int SUP_blocks_amount;
	memcpy(&SUP_blocks_amount, sizeof(uint32_t)+superBloque, sizeof(uint32_t));

	char* DIR_Bloques=string_new();
	string_append(&DIR_Bloques,obtener_directorio("/Blocks.ims"));
	struct stat sb;
	stat(DIR_Bloques,&sb);
	int tamanio_blocks=sb.st_size; //obtengo los bytes que pesa el Blocks.ims
	int real_block_amount=tamanio_blocks/SUP_block_size;

	printf("blocks amount superbloque: %d, cantidad real de bloques: %d\n",SUP_blocks_amount,real_block_amount);

	if(SUP_blocks_amount!=real_block_amount) {
		log_info(logger, "Se saboteo el SIZE del archivo de SuperBloque. Solucionando sabotaje...");
		memcpy(superBloque+sizeof(int), &real_block_amount, sizeof(uint32_t));
		msync(superBloque, sizeof(int)*2, MS_SYNC);
		close(superBlock_file);
		free(DIR_SuperBLoque);
		log_info(logger, "Sabotaje solucionado");
		return;
	}

	close(superBlock_file);
	free(DIR_Bloques);
	free(DIR_SuperBLoque);
	//liberar sb??
}

void arreglar_Bitmap_superBloque(){
	char* DIR_SuperBLoque=string_new();
	string_append(&DIR_SuperBLoque,obtener_directorio("/superBloque.ims"));
	int superBlock_file = open(DIR_SuperBLoque, O_CREAT | O_RDWR, 0664);
	int bitmap_size = roundUp(blocks_amount,8);
    void* superBloque = mmap(NULL, 8, PROT_READ | PROT_WRITE, MAP_SHARED, superBlock_file, 0);
    t_bitarray* bitmap_copy;

    //memcpy(&bitmap_copy, 2*(sizeof(uint32_t))+superBloque,bitmap_size);

    bitmap_copy = bitarray_create_with_mode((char*) superBloque+sizeof(int)+sizeof(int), bitmap_size, MSB_FIRST);

    char* bitmap_vacio = string_new();

    for(int i = 0; i < bitmap_size; i++)
    	string_append(&bitmap_vacio,"0");

    t_bitarray* bitmap_correcto = bitarray_create_with_mode(bitmap_vacio, bitmap_size, MSB_FIRST);
    free(bitmap_vacio);

    char*Dir_metadata=obtener_directorio("/Files/Oxigeno.ims");
    calcularBloquesUsadosRecursos(bitmap_correcto,Dir_metadata);
    free(Dir_metadata);
    Dir_metadata=obtener_directorio("/Files/Comida.ims");
    calcularBloquesUsadosRecursos(bitmap_correcto,Dir_metadata);
    free(Dir_metadata);
    Dir_metadata=obtener_directorio("/Files/Basura.ims");
	calcularBloquesUsadosRecursos(bitmap_correcto,Dir_metadata);
	free(Dir_metadata);

	for(int i = 0; i < list_size(lista_tripulantes); i++) {
	        tripulante* trip = (tripulante*)list_get(lista_tripulantes, i);
	        calcularBloquesUsadosRecursos(bitmap_correcto,trip->dir_bitacora);
	    }
	 //imprimir_bitmap(bitmap_correcto);
	bool esElMismo=true;
	for(int i = 0; i<bitmap_size; i++){
		if(bitarray_test_bit(bitmap_copy, i) == bitarray_test_bit(bitmap_correcto, i)){
			esElMismo=false;
			break;
		}

	}
	if(!esElMismo){
		bitmap_copy=bitmap_correcto;
		msync(bitmap_copy->bitarray,bitmap_size,MS_SYNC);
		msync(superBloque, sizeof(uint32_t)*2 + bitmap_size, MS_SYNC);
	}




    //if(SUP_blocks_amount!=real_block_amount){
    	log_info(logger, "Se saboteo el SIZE del archivo de SuperBloque. Solucionando sabotaje...");
    	// memcpy(superBloque+sizeof(int), &real_block_amount, sizeof(int));
    	 msync(superBloque, sizeof(int)*2, MS_SYNC);
    	 close(superBlock_file);
    	 free(DIR_SuperBLoque);
    	 log_info(logger, "Sabotaje solucionado");
    	 return;
 //   }
    close(superBlock_file);
    free(DIR_SuperBLoque);
    //liberar sb??
}
//-------------------------------------------------Sabotajes a FILES-----------------------------------------------------------
/*
void arreglar_blocks_recursos(char* DIR_metadata) {
	 t_config* metadata=config_create(DIR_metadata);
	 char** bloques=config_get_array_value(metadata,"BLOCKS");
	 int size=config_get_int_value(metadata,"SIZE");
	 char* string_caracter_llenado=config_get_string_value(metadata,"CARACTER_LLENADO");
	 char caracter_llenado=string_caracter_llenado[0];
	 char** bloques_validos=malloc(roundUp(size,block_size));
	 int aux=0;

	 size % block_size;

	 for(int i=0;i<config_get_int_value(metadata,"BLOCK_COUNT");i++){
		 if(atoi(bloques[i])<config_get_int_value(config,"BLOCKS_AMOUNT") || atoi(bloques[i])>=0){ //si el numero de bloque es mayor a la cantidad de bloques, o menor a 0
			 log_info(logger, "Se encontro un bloque perteneciente al File System");
			 bloques_validos[aux]=bloques[i];
			 aux++;
		 }
	 }
	 if(aux != config_get_int_value(config,"BLOCKS_AMOUNT") && (size%block_size)!=0){
		 int caract_faltantes=size-(aux-1)*block_size; //como el ultimo bloque no se si esta vacio o lleno, no lo tomo en cuenta y lo vuelvo a llenar
		 escribir_caracter_en_bloque(caracter_llenado,); terminar
	 }
}*/
void arreglar_size_recursos(char* DIR_metadata){
	t_config* metadata=config_create(DIR_metadata);
	 char** bloques=config_get_array_value(metadata,"BLOCKS");
	 int size_encontrado=config_get_int_value(metadata,"SIZE");
	 int cantidad_bloques=config_get_int_value(metadata,"BLOCK_COUNT");
	 char* string_caracter_llenado=config_get_string_value(metadata,"CARACTER_LLENADO");
	 char caracter_llenado=string_caracter_llenado[0];

	 int size_correcto=(cantidad_bloques-1)*block_size; //el ultimo debo revisar cuantos caracteres tiene escritos
	 char caracter=caracter_llenado;
	 int bloque_a_vereificar= atoi(bloques[cantidad_bloques]);
	 int aux=0;
	// for(int=0;i<block_size;i++){
	 while(caracter==caracter_llenado){
		 int desplazamiento =bloque_a_vereificar * block_size + aux;
		 memcpy(&caracter, blocks_copy + desplazamiento, sizeof(char));
		 aux++;
	 }
	size_correcto+=aux;
	if(size_encontrado != size_correcto){
		 log_info(logger, "Se saboteo el SIZE del archivo de recursos. Solucionando sabotaje...");
		 char* str_size_correcto=string_itoa(size_correcto);
		 config_set_value(metadata,"SIZE",str_size_correcto);
		 log_info(logger, "Sabotaje solucionado");
		 free(str_size_correcto);
	}


}
void arreglar_blockcount_recursos(char* DIR_metadata) {
	t_config* metadata=config_create(DIR_metadata);
	 char** bloques=config_get_array_value(metadata,"BLOCKS");
	 int block_count_encontrado=config_get_int_value(metadata,"BLOCK_COUNT");

	 int bloques_correctos=0;
	 while(bloques[bloques_correctos] != NULL){
		 bloques_correctos++;
	 }
	 if(bloques_correctos != block_count_encontrado){
		 log_info(logger, "Se saboteo la cantidad de bloques del archivo de recursos. Solucionando sabotaje...");
		 char* str_bloques_correctos=string_itoa(bloques_correctos);
		 config_set_value(metadata,"BLOCK_COUNT",str_bloques_correctos);
		 log_info(logger, "Sabotaje solucionado");
		 free(str_bloques_correctos);
	 }
	//config_set_value(metadata,"SIZE",size_correcto);
}
