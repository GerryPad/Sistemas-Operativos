#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H
#include <stdbool.h>
#include "nodo.h"

typedef struct {
    char *nombre;
    int valor;
} Registro; 

//Prototipos de funciones que llama directamente el main.
Registro* buscaRegistro(char *nombre);
bool validarToken(char *arr[], char *tok);
bool ejecOperacion(char *instruccion, char *args, struct Nodo *nodo, int *ptr_pc, int *ptr_pid);
bool instEND();
bool instJNZ(char *args, struct Nodo *nodo, int *ptr_pc, int *ptr_pid);
int interpretar_comando(char *comando, char *archivo, int *ptr_pid, int *ptr_inst);

//Variables que usa el main
extern char *delimitadores;
extern char *instruccion[];
extern Registro registros[];

#endif