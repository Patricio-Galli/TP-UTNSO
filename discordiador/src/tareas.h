#ifndef TAREA_H_
#define TAREA_H_

#include <stdlib.h>

#include <commons/string.h>

#include "tripulante.h"

typedef enum {
	GENERAR_OXIGENO,
	CONSUMIR_OXIGENO,
	GENERAR_COMIDA,
	CONSUMIR_COMIDA,
	GENERAR_BASURA,
	DESCARTAR_BASURA,
	ESPERAR
}tareas;

void ejecutar(char*, tripulante*);
tareas stringToEnum(char *);

#endif /* TAREA_H_ */
