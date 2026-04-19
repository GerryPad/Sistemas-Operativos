#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H
#include <stdbool.h>

typedef struct {
    char *nombre;
    int valor;
} Registro; 

//Prototipos de funciones que llama directamente el main.
Registro* buscaRegistro(char *nombre);
bool validarToken(char *arr[], char *tok);
bool ejecOperacion(char *instruccion, char *argumentos);
bool instEND();
int interpretar_comando(char *comando, char *archivo, int *ptr_pid);

//Variables que usa el main
extern char *instruccion[];
extern Registro registros[];

#endif