#ifndef DISPATCH_H
#define DISPATCH_H
#include "nodo.h"
#include "instrucciones.h"
#include "ram.h"
#include "ncurses.h"

#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768
#define TOTAL_INSTRUCCIONES_POR_MARCO 4
void guardaPCB(struct Nodo *PCB, int pc, char *linea);
int restauraPCB(struct Nodo *proceso_actual, char *archivo);
void aumentaGCPU(struct Nodo *listos, int gid);
int contarGrupos(struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, int max_gid);
void calculoPrioridades(struct Nodo *listos, struct Nodo *suspendidos, int grupos);
struct Nodo* planificador(struct Nodo *listos, struct Nodo *ejecutando);
void actualizaCGPU(struct Nodo *aux);
struct TablaMarcos *sacarSuspendidos(struct Nodo *suspendidos, struct Nodo *listos, struct TablaMarcos *manecilla_reloj, struct Nodo *ejecutando, struct TablaMarcos *tmm, struct TablaMarcos *tms, char *RAM, FILE *bin);
void sacarNuevos(struct Nodo *nuevos, struct Nodo *listos, TablaMarcos *tms, FILE *bin);
#endif