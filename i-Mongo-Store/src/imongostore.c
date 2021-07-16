#include "imongostore.h"

uint32_t* block_size;
uint32_t* blocks_amount;

int main(void)
{

	t_log* logger = log_create("imongostore.log", "I-MONGO-STORE", 1, LOG_LEVEL_DEBUG);


	t_config* config = config_create("imongostore.config");
	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
	char DIR_files[100];
	char DIR_bitacoras[100];
	char DIR_superBloque[100];
	char DIR_oxigeno[100];
	char DIR_comida[150];
	char DIR_basura[150];
	char DIR_tripulante[150];

	log_info(logger, "toy aqui");
	//reviso si existe el archivo
	if(!access(punto_montaje,R_OK)){
		log_info(logger, "Ya esta creado el archivo");
		//que hago si existe?
	}
	else{
		log_info(logger, "Generando File System limpio");
		printf("El punto de montajes es %s\n",punto_montaje);


		mkdir(punto_montaje,0755); //se crea el directorio /FileSystem/
		printf("el punto de montaje es %s\n",punto_montaje);

		char *directorio=DIR_files;
		strcpy(DIR_files,punto_montaje);
		strcat(DIR_files,"/Files");
		mkdir(directorio,0755); //se crea el directorio /FileSystem/Files
		printf("el directorio de files es %s\n",directorio);

		strcpy(DIR_bitacoras,DIR_files);
		strcat(DIR_bitacoras,"/Bitacoras");
		directorio=DIR_bitacoras;
		mkdir(directorio,0755); //se crea el directorio /FileSystem/Files/Bitacoras
		printf("el directorio de bitacoras es %s\n",directorio);

		//creo el superBloque

		strcpy(DIR_superBloque,punto_montaje);
		strcat(DIR_superBloque,"/superBloque.ims");
		directorio=DIR_superBloque;
		printf("el directorio del superbloque es %s\n",directorio);
		crear_superBloque(DIR_superBloque,16);

		//creo los archivos de metadata para oxigeno, comida y basura

		strcpy(DIR_oxigeno,DIR_files);
		strcat(DIR_oxigeno,"/Oxigeno.ims");
		directorio=DIR_oxigeno;
		crear_metadata(directorio);

		strcpy(DIR_comida,DIR_files);
		strcat(DIR_comida,"/Comida.ims");
		directorio=DIR_comida;
		crear_metadata(directorio);
		/*
		strcpy(DIR_basura,DIR_files);
		strcat(DIR_basura,"/Basura.ims");
		caca=DIR_basura;
		crear_metadata(caca);
*/

		/*if(!access(punto_montaje,R_OK)){
			log_info(logger, "Ya esta creado el archivo");
			//que hago si ya existe?
		}
		else
			log_info(logger, "Generando Super Bloque");
			int bitmap_size= (*blocks_amount/8);
			int superBloque_size=bitmap_size+2*sizeof(uint32_t);
			void* puntero_SuperBloque=crear_superBloque(DIR_superBloque,bitmap_size); //le paso bitmap_size para no calcularlo 2 veces
			munmap(puntero_SuperBloque,superBloque_size); //munmap(memoria a unmapear, bytes a unmapear
			log_info(logger, "ahora aca");
	}
*/
	log_info(logger, "Creando conexiones");

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
						/*recv(socket_discord, &cod_op, sizeof(int), MSG_WAITALL);
						log_info(logger, "Hemos vencido al recibir mensaje : %d", cod_op);
						recv(socket_discord, &cod_op, sizeof(int), MSG_WAITALL);
						log_info(logger, "Hemos vencido al recibir mensaje : %d", cod_op);*/
						break;
					}
				return EXIT_SUCCESS;
			}
}
}


char* crear_superBloque(char* DIR_superBloque,int bitmap_size){

	int superBloque_size=(bitmap_size+ 2*sizeof(uint32_t));
	printf("el tamaño del super bloque es %d\n",superBloque_size);
	//Crear bitmap?
	printf("el directorio del superBloque es %s\n",DIR_superBloque);
	FILE* fp;
	fp = fopen(DIR_superBloque,"w+");
	if(fp==NULL){
		printf("no se pudo abrir/generar el archivo\n");
	}
	char* puntero_SuperBloque;
	printf("se creo el super bloque\n");
	int tamanio_de_pagina=sysconf(_SC_PAGESIZE);
	printf("el tamanio de pag es %d\n",tamanio_de_pagina);


	/*
	puntero_SuperBloque=mmap(NULL,superBloque_size,PROT_WRITE,MAP_SHARED,fp,0);
	printf("Llegue aqui\n");
	if(puntero_SuperBloque == MAP_FAILED){
		printf("Se produjo un error mapeando el super bloque\n");
		return 0;
	}
	else
	{
		printf("Todo correcto por aca");

	else
	{
		log_info(logger, "Super Bloque mapeado con exito");
		memcpy(puntero_SuperBloque,&block_size, sizeof(uint32_t)); //memcopy(destino, origen, N°Bytes) copio block_size
		memcpy((puntero_SuperBloque+sizeof(uint32_t)),&blocks_amount, sizeof(uint32_t)); //copio blocks_amount pero desplazado por block_size
	//	memcpy((puntero_SuperBloque+2*sizeof(uint32_t)),&bitmap, bitmap_size);

	}

	}*/
	close(fp);
	return puntero_SuperBloque;
}

void crear_metadata(char* DIR_metadata){

	printf("el directorio de la metadata es %s\n",DIR_metadata);
	FILE* metadata;

	metadata=fopen(DIR_metadata,"wt");

	t_config* temp=config_create(DIR_metadata);
	temp->path=DIR_metadata;
	config_save_in_file(temp,DIR_metadata);

	config_set_value(temp,"SIZE","123");
	config_set_value(temp,"BLOCK_COUNT","4");
	config_set_value(temp,"BLOCKS","[5,6,7,8]");
	config_set_value(temp,"CARACTER_LLENADO","O");
	config_set_value(temp,"MD5_ARCHIVO","MEUDEUS");
	config_save(temp);

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
char* generar_directorio(char* nombre){

}
