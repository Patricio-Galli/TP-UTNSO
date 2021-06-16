/*
 * utils-conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/collections/list.h>
#include<commons/log.h>

typedef enum {
	INIT_P,
	INIT_T,
	DATA_T,
	ELIM_T,
	NEW_PO,
	ACTU_T,
	NEXT_T
}protocolo_msj;
// OP_CODE
// INIT_P
// INIT_T
// DATA_T
// ELIM_T 
// NEW_PO

// ACTU_T 
// NEXT_T

typedef enum {
	ENTERO,
	BUFFER
} tipo_msj;


typedef struct {
	int tamanio;
	void* contenido;
} t_buffer;

typedef struct {
	int codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	protocolo_msj op_code;
	t_buffer* buffer;
} t_mensaje;


void* serializar_paquete(t_paquete* paquete, int bytes);


char* recibir_buffer(int* size, int socket);
int recibir_operacion(int);
void eliminar_paquete(t_paquete* paquete);

t_paquete* crear_paquete(void);

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);

void agregar_parametro_a_mensaje(t_mensaje* mensaje, void* parametro, int tipo_parametro, t_log* logger);
t_mensaje* crear_mensaje(protocolo_msj cod_op);
void enviar_mensaje(int socket, t_mensaje* mensaje);

// protocolo_msj recibir_mensaje(int socket, t_list* lista_parametros);
t_list* recibir_mensaje(int socket);
#endif /* UTILS_CONEXIONES_H_ */