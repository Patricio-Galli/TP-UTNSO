#ifndef MONGO_H_
#define MONGO_H_

#include "blocks.h"
#include "global.h"
#include "sabotajes.h"
#include "utilidades.h"
#include "bitacora.h"

bool crear_superBloque();
bool crear_blocks();
void* uso_blocks();
void crear_metadata(char*, char);

char* crear_bitacora(int id_trip, int id_patota);
void generar_directorio(char*);
char* obtener_directorio(char*);
void imprimir_bitmap(t_bitarray*);
void* detector_sabotajes(void* s);
void* rutina_trip(void* t);
void* detector_sabotajes(void* s);
void* rutina_trip(void* t);
char** obtener_bitacora(int id_trip, int id_patota);
void liberar_split(char** split);

#endif /* MONGO_H_ */
