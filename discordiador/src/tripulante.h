#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <stdlib.h>

#include "global.h"
#include "planificador.h"
#include "mensajes_trip.h"
#include "validaciones.h"

tripulante* crear_tripulante(int, int, int, int, int, int);
void* rutina_tripulante(void*);
bool ejecutar(char*, tripulante*);
bool moverse(tripulante*, int, int);
bool ejecutar_io(tripulante* trip, tareas tarea, int cantidad);
bool esperar(int, tripulante*);

#endif /* TRIPULANTE_H_ */
