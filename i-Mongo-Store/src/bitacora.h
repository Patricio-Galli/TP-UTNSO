#ifndef BITACORA_H_
#define BITACORA_H_

#include "global.h"
#include "utilidades.h"
#include "blocks.h"

void actualizar_posicion(tripulante*, int, int,char*);
void comienza_tarea(char*,char*);
void finaliza_tarea(char*,char*);
void inicio_sabotaje(char*);
void fin_sabotaje(char*);
int escribir_caracter_en_bitacora(char*,int,char*,int);
void escribir_mensaje_en_bitacora(char* , char* );

#endif /* BITACORA_H_ */
