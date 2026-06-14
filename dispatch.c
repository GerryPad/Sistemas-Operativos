#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "dispatch.h"
#include "swap.h"

//Guadar del "procesador" a los PCB de cada proceso
void guardaPCB(struct Nodo *PCB, int pc, char *linea){
    PCB->PC = pc; 
    strcpy(PCB->IR,linea);
    PCB->registros[0]=registros[0].valor;
    PCB->registros[1]=registros[1].valor;
    PCB->registros[2]=registros[2].valor;
    PCB->registros[3]=registros[3].valor;
}

//Pasa del PCB al "procesador", devuelve el PC en el que se quedo el proceso
int restauraPCB(struct Nodo *proceso_actual, char *archivo){
    registros[0].valor = proceso_actual->registros[0];
    registros[1].valor = proceso_actual->registros[1];
    registros[2].valor = proceso_actual->registros[2];
    registros[3].valor = proceso_actual->registros[3];
    strcpy(archivo, proceso_actual->archivo); 
    return proceso_actual->PC;
}

//Para incrementar el GCPU de los procesos en el mismo grupo, sin importar en que lista esten
void aumentaGCPU(struct Nodo *listos, int gid){
    struct Nodo *aux = listos->siguiente;

    while (aux != NULL) {
        if(aux->GID== gid){
            aux->GCPU= aux->GCPU+20;
        }
        aux = aux->siguiente;
    }

}

//Devuelve el numero de grupos activos, sin importar la lista en la que este
int contarGrupos(struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, int max_gid) {
    int contador = max_gid;  //Asumir que todos los grupos estan activos

    for (int i = 1; i <= max_gid; i++) {
        if (buscaGID(ejecutando, i) != NULL) {
            continue; //Si esta en ejecutando ya no lo busques en listos
        } else {
            if (buscaGID(listos, i) != NULL) {
                continue; //Si esta en listos ya no lo busques en suspendidos
            } else {
                //Si jamas lo encontraste, el grupo no esta activo
                if (buscaGID(suspendidos, i) == NULL) contador--; 
            }
        }
    }
    return contador;
}

//Divide el CPU y GCPU de los procesos en una lista, cada que hay fin de quantum
void actualizaCGPU(struct Nodo *aux){
    
    while(aux != NULL) {
        aux->CPU = aux->CPU/2;
        aux->GCPU = aux->GCPU/2;
        aux = aux->siguiente;
    }
}

//Determina la prioridad de cada proceso, actualiza los valores de CPU y GCPU antes de hacerlo
void calculoPrioridades(struct Nodo *listos, struct Nodo *suspendidos, int grupos) {
    struct Nodo *aux = listos->siguiente;
    struct Nodo *aux2 = listos->siguiente;
    struct Nodo *aux3 = suspendidos->siguiente;
    int p_base = 60; 

    actualizaCGPU(aux2);
    actualizaCGPU(aux3);
    
    while(aux!= NULL){
        aux->prioridad = p_base + (int) ((aux->CPU/2.0)) + (int) ((aux->GCPU * grupos/4.0));
        aux = aux->siguiente;
    }
}

//Itera sobre los listos y busca aquel de mayor prioridad (nuemero mas chico) y lo pone en ejecutando
struct Nodo *planificador(struct Nodo *listos, struct Nodo *ejecutando) {
    if (listos->siguiente == NULL){
        return NULL; //Caundo no hay nada en listos
    } 

    struct Nodo *aux = listos->siguiente; 
    struct Nodo *proceso_prioritario = aux; //Suponemos que el primer nodo es el que tiene mayor prioridad 

    while(aux != NULL){ 
        if(aux->prioridad < proceso_prioritario->prioridad){ 
            proceso_prioritario = aux; 
        }
        aux=aux->siguiente;
    }

    struct Nodo *proceso = extraerPID(listos, proceso_prioritario->PID); //"desencolamos" el proceso de mayor prioridad
    proceso_prioritario->estadoTermino = 0;
    insertarFinal(ejecutando, proceso);
    return proceso;
}

//Planificador a mediano plazo, comprueba si un suspendido ya ha pasado su tiempo o mas de espera
struct TablaMarcos *sacarSuspendidos(struct Nodo *suspendidos, struct Nodo *listos, struct TablaMarcos *manecilla_reloj, struct Nodo *ejecutando, struct TablaMarcos *tmm, struct TablaMarcos *tms, char *RAM, FILE *bin){
    struct Nodo *aux_s = suspendidos->siguiente;
    struct Nodo *proceso_a_mover = NULL;
    struct Nodo *siguiente_nodo = NULL;
    int pag_actual;
    int marco_ram_nuevo;

    while(aux_s != NULL){
        siguiente_nodo = aux_s->siguiente;
        if(difftime(time(NULL), aux_s->hora_entrada) >= aux_s->tiempo_espera) {
            pag_actual = aux_s->PC/INSTRUCCIONES_POR_MARCO;
           if(aux_s->tmp[pag_actual].num_marco_ram != -1){
                proceso_a_mover = extraerPID(suspendidos, aux_s->PID);
                if (proceso_a_mover != NULL) {
                    insertarFinal(listos, proceso_a_mover);
                }
            } else {
                manecilla_reloj = algoritmoReloj(manecilla_reloj, listos, ejecutando, suspendidos, tmm, RAM);
                marco_ram_nuevo = cargarARAM(aux_s->PID, aux_s->GID, pag_actual, bin, manecilla_reloj, listos, ejecutando, suspendidos, tms, tmm, RAM);
                porcentajeDiscoRAM(tmm, tms);
                actualizaTMP(aux_s, tms); 
                if (marco_ram_nuevo != -1) {
                    aux_s->tmp[pag_actual].num_marco_ram = marco_ram_nuevo;
                    tmm[marco_ram_nuevo].usado_recien = 1;
                    tmm[marco_ram_nuevo].puntero = false;
                }

                manecilla_reloj = manecilla_reloj->siguiente;
                for(int i = 0; i<TOTAL_MARCOS_RAM; i++){
                    tmm[i].puntero = false;
                }
                manecilla_reloj->puntero = true;
                proceso_a_mover = extraerPID(suspendidos, aux_s->PID);
                if (proceso_a_mover != NULL) {
                    insertarFinal(listos, proceso_a_mover);
                }
            }
        }
        aux_s = siguiente_nodo;
    }
    return manecilla_reloj;
}

//Planificador a largo plazo, comprueba si hay espacio en disco para cargarlo
void sacarNuevos(struct Nodo *nuevos, struct Nodo *listos, TablaMarcos *tms, FILE *bin){
    struct Nodo *aux_n=nuevos->siguiente;
    struct Nodo *proceso_a_mover=NULL;
    struct Nodo *siguiente_nodo = NULL;
    int libre=0;

    for(int i=0; i<TOTAL_MARCOS_DISCO; i++){
        if(tms[i].propietario == 0){
            libre++;
        }
    }
    while(aux_n !=NULL){
        siguiente_nodo = aux_n->siguiente;
        if(aux_n->num_paginas<=libre){
            proceso_a_mover=extraerPID(nuevos,aux_n->PID);
            if(proceso_a_mover != NULL){
                insertarFinal(listos,proceso_a_mover);
                guardarTextoABinario(proceso_a_mover->archivo, bin, proceso_a_mover->PID, proceso_a_mover->GID, tms);     
                libre = libre - proceso_a_mover->num_paginas;
            }
            
        }
        aux_n = siguiente_nodo;
    }

}