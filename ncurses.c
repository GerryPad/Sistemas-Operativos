#include <stdio.h>
#include <curses.h>
#include "ncurses.h"

void imprimir_registros(int renglon, char *instruccion){
    mvprintw(3, 2, "%-8s %-15s %-20s %-20s %-20s %-20s", 
        "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    move(5,2);
    clrtoeol();
    mvprintw(5, 2, "%-8d %-15s %-20d %-20d %-20d %-20d", 
        renglon, 
        instruccion, 
        registros[0].valor, 
        registros[1].valor, 
        registros[2].valor, 
        registros[3].valor  
    );
    refresh();
}

void limpia_lineas() {
    for (int i = 34; i<= 40 ; i++) {
        move(i,2); clrtoeol();    
    }
    refresh();
}

//Imprimir primero el ejecutando, despues los listos en el orden que estan en listos y finalmente los terminados en su orden
void imprimir_listas(struct Nodo *cabecera_ejecutando, struct Nodo *cabecera_listos, struct Nodo *cabecera_terminados, struct Nodo *cabecera_suspendidos, struct Nodo *cabecera_nuevos){
    struct Nodo *aux_e = cabecera_ejecutando->siguiente;
    struct Nodo *aux_l = cabecera_listos->siguiente;
    struct Nodo *aux_te = cabecera_terminados->siguiente;
    struct Nodo *aux_n = cabecera_nuevos->siguiente;
    struct Nodo *aux_s = cabecera_suspendidos->siguiente;

    mvprintw(7, 2, "%-6s %-6s %-8s %-14s %-8s %-15s %-20s %-20s %-20s %-20s %-8s %-8s %-8s", 
        "PID", "GID", "File", "Estatus", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU", "GCPU", "Prioridad");


    for(int j = 8; j<34; j++){
        move(j,2);
        clrtoeol();
    } 
 //Esta es la lista de ejecutando
 //La impresion ahora es mas bonita 
   int i = 8;
    if(aux_e != NULL){
        mvprintw(i, 2,"%-6d %-6d %-8s %-14s %-8s %-15s %-20s %-20s %-20s %-20s %-8d %-8d %-8d", 
        aux_e->PID,
        aux_e->GID,
        aux_e->archivo,
        "ejecutando",
        "---",
        "---",
        "---",
        "---",
        "---",
        "---", 
        aux_e->CPU,
        aux_e->GCPU,
        aux_e->prioridad
        );
        i++;
    }

    //Lista de listos
    while(aux_l != NULL){
        if(i>=34 || aux_l == NULL){
            break;
        }

        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-6d %-8s %-14s %-8d %-15s %-20d %-20d %-20d %-20d %-8d %-8d %-8d", 
        aux_l->PID,
        aux_l->GID,
        aux_l->archivo,
        "listos",
        aux_l->PC,
        aux_l->IR,
        aux_l->registros[0],
        aux_l->registros[1],
        aux_l->registros[2],
        aux_l->registros[3],
        aux_l->CPU,
        aux_l->GCPU,
        aux_l->prioridad
        );

        aux_l = aux_l->siguiente;
        i++;
    }

    //Lista de nuevos
    //int i=9;
    while(aux_n != NULL){
        if(i>=34 || aux_n == NULL){
            break;
        }

        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-6d %-8s %-14s %-8d %-15s %-20d %-20d %-20d %-20d %-8d %-8d %-8d", 
        aux_n->PID,
        aux_n->GID,
        aux_n->archivo,
        "nuevos",
        aux_n->PC,
        aux_n->IR,
        aux_n->registros[0],
        aux_n->registros[1],
        aux_n->registros[2],
        aux_n->registros[3],
        aux_n->CPU,
        aux_n->GCPU,
        aux_n->prioridad
        );

        aux_n = aux_n->siguiente;
        i++;
    }

    //Lista de suspendidos
    while(aux_s != NULL){
        if(i>=34 || aux_s == NULL){
            break;
        }

        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-6d %-8s %-14s %-8d %-15s %-20d %-20d %-20d %-20d %-8d %-8d %-8d", 
        aux_s->PID,
        aux_s->GID,
        aux_s->archivo,
        "suspendidos",
        aux_s->PC,
        aux_s->IR,
        aux_s->registros[0],
        aux_s->registros[1],
        aux_s->registros[2],
        aux_s->registros[3],
        aux_s->CPU,
        aux_s->GCPU,
        aux_s->prioridad
        );

        aux_s = aux_s->siguiente;
        i++;
    }

    //Lista de terminados
    while(aux_te != NULL){
        if(i>=34 || aux_te == NULL){
            break;
        }
        move(i,2);
        clrtoeol();
        mvprintw(i, 2, "%-6d %-6d %-8s", 
        aux_te->PID,
        aux_te->GID,
        aux_te->archivo
        );
        if (aux_te -> estadoTermino == 0){
            mvprintw(i,25, "%-12s", "terminados");
        } else if(aux_te -> estadoTermino == 1) {
            mvprintw(i,25, "%-12s", "terminados*");
        } else if(aux_te -> estadoTermino == 2) {
            mvprintw(i,25, "%-12s", "terminados**");
        }
        //aux_te->estado, //Aqui va la logica de terminados con errorm normal o matados 
        mvprintw(i, 40, "%-8d %-15s %-20d %-20d %-20d %-20d %-8d %-8d %-8d",
        aux_te->PC,
        aux_te->IR,
        aux_te->registros[0],
        aux_te->registros[1],
        aux_te->registros[2],
        aux_te->registros[3],
        aux_te->CPU,
        aux_te->GCPU,
        aux_te->prioridad 
        );

        aux_te = aux_te->siguiente;
        i++;
    }
    refresh();
}