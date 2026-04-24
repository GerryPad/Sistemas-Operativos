#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dispatch.h"

void guardaPCB(struct Nodo *PCB, int pc, char *linea){
    PCB->PC = pc; //Sera que tenemos que guardar la siguiente instruccion o donde se quedo?
    strcpy(PCB->IR,linea);
    PCB->registros[0]=registros[0].valor;
    PCB->registros[1]=registros[1].valor;
    PCB->registros[2]=registros[2].valor;
    PCB->registros[3]=registros[3].valor;
}

//Esto debe devolver un entero para el pc
int restauraPCB(struct Nodo *proceso_actual, char *archivo){
    //pc = proceso_actual->PC
    registros[0].valor = proceso_actual->registros[0];
    registros[1].valor = proceso_actual->registros[1];
    registros[2].valor = proceso_actual->registros[2];
    registros[3].valor = proceso_actual->registros[3];
    strcpy(archivo, proceso_actual->archivo); 
    return proceso_actual->PC; //Se ocupaba hacer esto porque si no, jamas devolvia el valor
}

//El planificador primitivo, solo devuelve el primero que este en listos
struct Nodo* planificador(struct Nodo *listos, struct Nodo *ejecutando) {
    if (listos->siguiente == NULL){
        return NULL; //Caundo no hay nada en listos
    } 

    
    struct Nodo *proceso = desencolar(listos);
    strcpy(proceso->estado, "ejecutando");
    insertarFinal(ejecutando, proceso);
    return proceso;
}