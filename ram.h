#ifndef RAM_H
#define RAM_H
#include "nodo.h"
#define TAMANO_IR 64 
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) 
#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768

int cargarARAM(int pid, int gid, int num_pagina, FILE *bin, struct TablaMarcos *manecilla,struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, TablaMarcos *tms, TablaMarcos *tmm, char *RAM);
void eliminarPaginas(struct Nodo *proceso, TablaMarcos *tms, TablaMarcos *tmm, struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, FILE *bin, char *RAM);
struct Nodo* buscarHerederoGID(struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, int gid, int pid_actual);
struct TablaMarcos *algoritmoReloj(TablaMarcos *manecilla, struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, TablaMarcos *tmm, char *RAM);

#endif