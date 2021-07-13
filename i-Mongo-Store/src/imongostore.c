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
	log_info(logger, "toy aqui");
	//reviso si existe el archivo
	if(!access(punto_montaje,R_OK)){
		log_info(logger, "Ya esta creado el archivo");
		//que hago si existe?
	}
	else{
		log_info(logger, "Generando File System limpio");
		printf("El punto de montajes es %s\n",punto_montaje);

		char *directorio=DIR_files;
		strcpy(DIR_files,punto_montaje);
		strcat(DIR_files,"/Files");;
		mkdir(directorio,0755); //se crea el directorio /FileSystem/Files
		printf("el directorio de files es %s\n",directorio);

		strcpy(DIR_bitacoras,DIR_files);
		strcat(DIR_bitacoras,"/Bitacoras");
		directorio=DIR_bitacoras;
		mkdir(directorio,0755); //se crea el directorio /FileSystem/Files/Bitacoras
		printf("el directorio de bitacoras es %s\n",directorio);

		//creo el superBloque

		strcpy(DIR_superBloque,DIR_files);
		strcat(DIR_superBloque,"/superBloque.ims");
		directorio=DIR_superBloque;
		printf("el directorio del superBloque es %s\n",directorio);
		crear_superBloque(DIR_superBloque,16);

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


void* crear_superBloque(char* DIR_superBloque,int bitmap_size){

	int superBloque_size=(bitmap_size+ 2*sizeof(uint32_t));
	printf("el tamaño del super bloque es %d\n",superBloque_size);
	//Crear bitmap?

	FILE *fp; //lo necesito para el mmap
	fp = fopen(DIR_superBloque,"wb");
	void* puntero_SuperBloque;
	printf("se creo el super bloque\n");
	long tamanio_de_pagina=sysconf(_SC_PAGESIZE);
	printf("el tamanio de pag es %d\n",tamanio_de_pagina);
	/*
	puntero_SuperBloque=mmap(NULL,superBloque_size,PROT_WRITE,MAP_SHARED,fp,0);

	if(puntero_SuperBloque == -1){
		log_info(logger, "Se produjo un error mapeando el super bloque");
	}

	else
	{
		log_info(logger, "Super Bloque mapeado con exito");
		memcpy(puntero_SuperBloque,&block_size, sizeof(uint32_t)); //memcopy(destino, origen, N°Bytes) copio block_size
		memcpy((puntero_SuperBloque+sizeof(uint32_t)),&blocks_amount, sizeof(uint32_t)); //copio blocks_amount pero desplazado por block_size
	//	memcpy((puntero_SuperBloque+2*sizeof(uint32_t)),&bitmap, bitmap_size);

	}
	*/
	fclose(fp);
	return puntero_SuperBloque;

}
//*/
