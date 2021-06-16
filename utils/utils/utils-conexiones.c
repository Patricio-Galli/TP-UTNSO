#include "utils-conexiones.h"
/*
void* serializar_paquete(t_paquete* paquete, int bytes) {
	void * magic = malloc(bytes);
	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

*/
void crear_buffer(t_mensaje* mensaje) {
	mensaje->buffer = malloc(sizeof(t_buffer));
	mensaje->buffer->tamanio = 0;
	mensaje->buffer->contenido = NULL;
}
/*
t_paquete* crear_paquete(void) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}
*/
t_mensaje* crear_mensaje(protocolo_msj cod_op) {
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	// memcpy(&(mensaje->op_code), &cod_op, sizeof(int));
	mensaje->op_code = cod_op;
	crear_buffer(mensaje);
	return mensaje;
}

void agregar_parametro_a_mensaje(t_mensaje* mensaje, void* parametro, int tipo_parametro, t_log* logger) {
	switch (tipo_parametro) {
	case ENTERO:
		log_info(logger, "case: ENTERO tamanio = %d", mensaje->buffer->tamanio);
		log_info(logger, "Parametro %d", *(int *)parametro);
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(int));
		// log_info(logger, "Pase realloc, %d", sizeof(*(mensaje->buffer->contenido)));
		
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, parametro, sizeof(int));
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(int);
		log_info(logger, "memcpy del valor: tamanio = %d", mensaje->buffer->tamanio);
		break;
	case BUFFER:
		log_info(logger, "case: BUFFER tamanio = %d. Parametro %s", mensaje->buffer->tamanio, parametro);
		int tamanio_buffer = (strlen(parametro) + 1) * sizeof(char);
		// memcpy(&(tamanio_buffer), &tamanio_buffer, sizeof(int));
		// log_info(logger, "hice el memcpy");
		// char* buffer = malloc(sizeof(char) * tamanio_buffer);
		// memcpy(buffer, parametro, tamanio_buffer);
		// log_info(logger, "asigne variables");
		mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(int) + tamanio_buffer);
		log_info(logger, "realloc");
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &tamanio_buffer, sizeof(int));
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio + sizeof(int), parametro, tamanio_buffer);
		log_info(logger, "memcpy's");
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(int) + tamanio_buffer;
		log_info(logger, "%s, %d", parametro, tamanio_buffer);
		log_info(logger, "break");
		break;
	default:
		log_info(logger, "Guarda, estoy en deafult");
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
	recv(socket, &op_code, sizeof(int), MSG_WAITALL);
	list_add(lista_parametros, (void *)op_code);
	printf("OP CODE agregado. Valor: %d\n", op_code);
	
	int id_patota, cant_tripulantes, cant_tareas;
	switch(op_code) {
	case INIT_P:
		id_patota = (int)recibir_parametro(socket, ENTERO);
		list_add(lista_parametros, (void *)id_patota);
		printf("Id patota agregado. Valor %d\n", id_patota);

		cant_tripulantes = (int)recibir_parametro(socket, ENTERO);
		list_add(lista_parametros, (void *)cant_tripulantes);
		printf("cant trip agregado. Valor %d\n", cant_tripulantes);
		
		cant_tareas = (int)recibir_parametro(socket, ENTERO);
		list_add(lista_parametros, (void *)cant_tareas);
		printf("cant tareas agregado. Valor %d\n", cant_tareas);
		
		for(int iterador = 0; iterador < cant_tareas; iterador++) {
			char * tarea = recibir_parametro(socket, BUFFER);
			list_add(lista_parametros, (void *)tarea);
			printf("Tarea agregada: %s\n", tarea);
		}
		break;
	case INIT_T:
		break;
	case DATA_T:
		break;
	case ELIM_T:
		break;
	case NEW_PO:
		break;
	case ACTU_T:
		break;
	case NEXT_T:
		break;
	default:
		printf("default !!!\n");
	}
	return lista_parametros;
}
/*
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_mensaje(char* mensaje, int socket_cliente) {
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}


char* recibir_buffer(int* size, int socket_cliente) {
	void * buffer;
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	printf("Recivo op: %d\n", *size);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	//printf("Recivo buffer: %s\n", (char*)buffer);
	return buffer;
}

char* recibir_mensaje(int socket_cliente) {
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego el mensaje %s\n", buffer);
	return buffer;
}

int recibir_operacion(int socket_cliente) {
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_list* recibir_paquete(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

*/