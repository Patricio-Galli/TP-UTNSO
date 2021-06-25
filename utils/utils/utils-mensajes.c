#include "utils-mensajes.h"

void crear_buffer(t_mensaje* mensaje) {
	mensaje->buffer = malloc(sizeof(t_buffer));
	mensaje->buffer->tamanio = 0;
	mensaje->buffer->contenido = NULL;
}

t_mensaje* crear_mensaje(protocolo_msj cod_op) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->op_code = cod_op;
	crear_buffer(mensaje);
	return mensaje;
}

void agregar_parametro_a_mensaje(t_mensaje* mensaje, void* parametro, tipo_msj tipo_parametro) {
	int valor_parametro;
	int tamanio_buffer;
	switch (tipo_parametro) {
	case ENTERO:
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(int));
		valor_parametro = (int)parametro;
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &valor_parametro, sizeof(int));
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(int);
		break;
	case BUFFER:
		tamanio_buffer = (strlen(parametro) + 1) * sizeof(char);
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(int) + tamanio_buffer);
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &tamanio_buffer, sizeof(int));
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio + sizeof(int), parametro, tamanio_buffer);
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(int) + tamanio_buffer;
		break;
	}
}

void enviar_mensaje(int socket, t_mensaje* mensaje) {
	int tamanio_buffer = mensaje->buffer->tamanio + sizeof(int);
	void * buffer_to_send = malloc(tamanio_buffer);
	memcpy(buffer_to_send, &(mensaje->op_code), sizeof(int));
	memcpy(buffer_to_send + sizeof(int), mensaje->buffer->contenido, mensaje->buffer->tamanio);
	send(socket, buffer_to_send, tamanio_buffer, 0);
	free(buffer_to_send);
}

void* recibir_parametro(int socket, tipo_msj tipo) {
	int parametro;
	int tamanio_buffer;
	switch (tipo) {
	case ENTERO:
		recv(socket, &parametro, sizeof(int), MSG_WAITALL);
		return (void *)parametro;
		break;
	case BUFFER:
		recv(socket, &tamanio_buffer, sizeof(int), MSG_WAITALL);
		char *buffer = malloc(tamanio_buffer * sizeof(char));
		recv(socket, buffer, tamanio_buffer, MSG_WAITALL);
		return buffer;
		break;
	}
	return NULL;
}

t_list* recibir_mensaje(int socket) {
	protocolo_msj op_code;
	t_list* lista_parametros = list_create();
	int pepe;
	pepe = recv(socket, &op_code, sizeof(int), MSG_WAITALL);
	if(pepe == 0) {
		printf("DOY 0\n");
		return NULL;
	}
	if(pepe == -1) {
		printf("DOY NULL\n");
		return NULL;
	}	
	list_add(lista_parametros, (void *)op_code);
	
	switch(op_code) {
	case INIT_P:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));

		int cant_tareas = (int)list_get(lista_parametros, 3);
		for(int iterador = 0; iterador < cant_tareas; iterador++) {
			list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		}
		break;
	
	case INIT_T:
	case SABO_P:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	
	case SHOW_T:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	
	case DATA_T:
	case ELIM_T:
	case ACTU_T:
	case BITA_D:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	
	case BITA_C:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		int cant_lineas = (int)list_get(lista_parametros, 1);
		for(int iterador = 0; iterador < cant_lineas; iterador++) {
			list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		}
		break;
	
	case SND_PO:
		list_add(lista_parametros, recibir_parametro(socket, ENTERO));
		break;
	
	case TASK_T:
		list_add(lista_parametros, recibir_parametro(socket, BUFFER));
		break;

	case NEW_PO:
	case NEXT_T:
	case TODOOK:
	case NO_SPC:
	case ER_MSJ:
		break;
	
	case BITA_T:
		// por definir
		break;
	
	case TAR_ES:
		// por definir
		break;
	
	default:
		// return NULL;
		break;
	}
	return lista_parametros;
}

void liberar_mensaje(t_mensaje* mensaje) {
	free(mensaje->buffer->contenido);
	free(mensaje->buffer);
	free(mensaje);
}