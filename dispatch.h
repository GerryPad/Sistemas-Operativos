#ifndef DISPATCH_H
#define DISPATCH_H
#include "nodo.h"
#include "instrucciones.h"

void guardaPCB(struct Nodo *PCB, int pc, char *linea);
int restauraPCB(struct Nodo *proceso_actual, char *archivo);
void aumentaGCPU(struct Nodo *listos, int gid);
int contarGrupos(struct Nodo *listos, struct Nodo *ejecutando, int max_gid);
void calculoPrioridades(struct Nodo *listos, int grupos);
struct Nodo* planificador(struct Nodo *listos, struct Nodo *ejecutando);

#endif