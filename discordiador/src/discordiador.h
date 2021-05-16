/*
 * discordiador.h
 *
 *  Created on: 13 may. 2021
 *      Author: utnso
 */

#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<utils/utils.h> // Cambiar nombre a utils mas adelante

//#include "utils.h"

#endif /* DISCORDIADOR_H_ */
t_log* iniciar_logger(void);
t_config* leer_config(void);
void leer_consola(t_log* logger);
t_paquete* armar_paquete();
#endif /* TP0_H_ */
