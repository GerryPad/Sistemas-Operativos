#ifndef SWAP_H
#define SWAP_H
//#include <stdbool.h>
//#include "instrucciones.h"
#include "nodo.h"

//Prototipos de funcion
int guardarTextoABinario(const char *archivoTexto, FILE *bin, int pid, int gid, TablaMarcos *tms);
int cuentaMarcosNecesarios(const char *nombre_archivo);
bool verificarEspacioEnSwap(const char *nombre_archivo, TablaMarcos *tms);
void iniciarDiscoYTablas(TablaMarcos *tms, TablaMarcos *tmm, FILE *bin);
#endif