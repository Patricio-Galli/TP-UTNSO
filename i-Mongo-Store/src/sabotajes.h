#ifndef SABOTAJES_H_
#define SABOTAJES_H_

#include "global.h"
#include "mongo.h"

char** ubicaciones_sabotajes;
int socket_sabotajes, contador_sabotajes;

void inicializar_detector_sabotajes(int socket_discord);
void analizador_sabotajes(int senial);

#endif /* SABOTAJES_H_ */
