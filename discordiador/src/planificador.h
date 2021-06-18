#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void planificador(int, char*, int, int, bool*, t_log*);

void* fifo();
void* rr();

#endif /* PLANIFICADOR_H_ */
