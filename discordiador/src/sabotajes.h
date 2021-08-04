#ifndef SABOTAJES_H_
#define SABOTAJES_H_

#include "global.h"
#include "planificador.h"

void emergency_trips_running();
void emergency_trips_ready();
tripulante* encontrar_mas_cercano(int pos_x, int pos_y);
void resolver_sabotaje(tripulante* trip, int pos_x, int pos_y, int socket_sabotajes);

#endif /* SABOTAJES_H_ */
