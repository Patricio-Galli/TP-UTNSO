#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/string.h>

#include <utils/utils-client.h>
#include <utils/utils-sockets.h>
#include <utils/utils-mensajes.h>

#define RAM_ACTIVADA 1
#define MONGO_ACTIVADO 0

//tripulante
typedef struct {
	int posicion[2];
	int id_trip;
	int id_patota;
	int estado; //ready, blocked, etc

	pthread_t hilo;

	int socket_ram;
	int socket_mongo;

	sem_t sem_blocked;
	sem_t sem_running;

	int contador_ciclos;
	int tiempo_esperado;
	bool quantum_disponible;
}tripulante;

//estado
typedef enum {
    NEW,
    BLOCKED,
    READY,
    RUNNING,
    EXIT,
	EMERGENCY
}estado;

//tareas
typedef enum {
	GENERAR_OXIGENO,
	CONSUMIR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA,
	ESPERAR
}tareas;

/////////////////////TRIPULANTE//////////////////////
tripulante* crear_tripulante(int, int, int, int, int, int);
void* rutina_tripulante(void*);
bool ejecutar(char*, tripulante*);
void moverse(tripulante*, int, int);
void ejecutar_io(tripulante* trip, tareas tarea, int cantidad);
bool esperar(int, tripulante*);


/////////////////////MENSAJES//////////////////////
bool respuesta_OK(t_list* respuesta, char* mensaje_fallo);
void enviar_y_verificar(t_mensaje* mensaje_out, int socket, char* mensaje_error);
char* solicitar_tarea(tripulante*);
void avisar_movimiento(tripulante*);
void actualizar_estado(tripulante* trip, estado estado_trip);


/////////////////////VALIDACIONES//////////////////////
void actualizar_quantum(tripulante*);
void puede_continuar(tripulante*);


/////////////////////UTILIDADES//////////////////////
char* estado_enumToString(int);
tareas stringToEnum(char *);
void liberar_input(char** input);

#endif /* TRIPULANTE_H_ */
