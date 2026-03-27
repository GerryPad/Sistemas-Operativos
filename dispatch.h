#ifndef DISPATCH_H
#define DISPATCH_H
#include "nodo.h"
#include "instrucciones.h"

void guardaPCB(struct Nodo *PCB, int pc, char *linea);
int restauraPCB(struct Nodo *proceso_actual, char *archivo);
struct Nodo* planificador(struct Nodo *listos, struct Nodo *ejecutando);

#endif