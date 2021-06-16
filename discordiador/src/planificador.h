#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <pthread.h>

void planificador(t_list*, int, char*, int, bool*, t_log*);

void* fifo();
void* rr();

/*
void iniciar_FIFO(int, t_list*, bool*, t_log*);
void iniciar_RR(int, int, t_list*, bool*, t_log*);
*/

#endif /* PLANIFICADOR_H_ */
