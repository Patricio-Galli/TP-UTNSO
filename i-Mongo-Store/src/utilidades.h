#ifndef UTILIDADES_H_
#define UTILIDADES_H_

#include "global.h"

char* crear_MD5(char, int);
tripulante* obtener_tripulante(int id_trip, int id_patota);
int roundUp(int, int);
char* juntar_posiciones(int, int);
void generar_directorio(char*);
char* obtener_directorio(char*);
void imprimir_bitmap(t_bitarray*);
void liberar_split(char** split);

#endif /* UTILIDADES_H_ */
