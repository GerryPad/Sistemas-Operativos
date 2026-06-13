#include <stdio.h>
#include <curses.h>
#include "ncurses.h"

#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768

//Muestra el proceso en ejecucion
void imprimir_registros(int renglon, char *instruccion){
    mvprintw(3, 2, "%-8s %-15s %-20s %-20s %-20s %-20s", 
        "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    move(4,2);
    clrtoeol();
    mvprintw(4, 2, "%-8d %-15s %-20d %-20d %-20d %-20d", 
        renglon, 
        instruccion, 
        registros[0].valor, 
        registros[1].valor, 
        registros[2].valor, 
        registros[3].valor  
    );
    refresh();
}

//Limpieza de avisos de errores
void limpia_lineas() {
    for(int j = 34; j<=40; j++){
        mvprintw(j, 2, "%-120s", "");
    } 
    refresh();
}

//En teoria esta no tiene que verse en pantalla, se usa solo para debug
void imprimirTms(TablaMarcos *tms){
    mvprintw(27, 135, "TMS");
    mvprintw(28,135,"%-6s %-6s %-6s", "Marco", "PID", "#Pagina" );
    int marco=0;
    for(int i=29; i<45; i++){
        mvprintw(i,135, "%-6d %-6d %-6d", tms[marco].num_marco, tms[marco].propietario, tms[marco].num_pagina);
    marco++;
    }
}

//Muestra lo que hay en cada marco de la RAM
void imprimirTmm(TablaMarcos *tmm){
    mvprintw(6, 163, "TMM");
    mvprintw(8, 163,"%-8s %-8s %-8s %-8s %-8s", "Marco", "PID", "GID", "#Pagina", "Usado?" );
    char *estado_puntero;

    int marco=0;
    for(int i=9; i<25; i++){
        if((tmm[marco].puntero == true)) {
            estado_puntero = "*";
        } else {
            estado_puntero = " ";
        }
        mvprintw(i,163, "%-8d %-8d %-8d %-8d %-8d %-3s", tmm[marco].num_marco, tmm[marco].propietario, tmm[marco].grupo, tmm[marco].num_pagina, tmm[marco].usado_recien, estado_puntero);
    marco++;
    }
}

//Muestra la TMP del proceso actual
void imprimirTmp(struct Nodo *proceso){
    for(int j=6; j<25; j++){
        mvprintw(j, 135, "%-28s", ""); 
    }
    mvprintw(6, 135, "TMP");
    mvprintw(8, 135,"%-8s %-8s %-8s", "Pagina", "MarcoRAM", "#MarcoD" );
    int marco=0;
    int aux = 9 + proceso->num_paginas;
    if (aux >= 25) aux=25;
    for(int i=9; i<aux; i++){
        mvprintw(i,135, "%-8d %-8d %-8d", proceso->tmp[marco].num_pagina, proceso->tmp[marco].num_marco_ram, proceso->tmp[marco].num_marco_disco);
        marco++;
    }
}

//Imprimir todas las listas
void imprimir_listas(struct Nodo *cabecera_ejecutando, struct Nodo *cabecera_listos, struct Nodo *cabecera_terminados, struct Nodo *cabecera_suspendidos, struct Nodo *cabecera_nuevos){
    struct Nodo *aux_e = cabecera_ejecutando->siguiente;
    struct Nodo *aux_l = cabecera_listos->siguiente;
    struct Nodo *aux_te = cabecera_terminados->siguiente;
    struct Nodo *aux_n = cabecera_nuevos->siguiente;
    struct Nodo *aux_s = cabecera_suspendidos->siguiente;

    mvprintw(6, 2, "%-4s %-4s %-7s %-13s %-6s %-12s %-11s %-11s %-11s %-11s %-8s %-8s %-8s", 
        "PID", "GID", "File", "Estatus", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU", "GCPU", "Prioridad");


    for(int j = 7; j<34; j++){
        mvprintw(j, 2, "%-100s", "");
    } 

   int i = 7;
   //Lista ejecutando
    if(aux_e != NULL){
        mvprintw(i, 2,"%-4d %-4d %-7s %-13s %-6s %-12s %-11s %-11s %-11s %-11s %-8d %-8d %-8d", 
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

        mvprintw(i, 2, "%-4d %-4d %-7s %-13s %-6d %-12s %-11d %-11d %-11d %-11d %-8d %-8d %-8d", 
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
    while(aux_n != NULL){
        if(i>=34 || aux_n == NULL){
            break;
        }

        mvprintw(i, 2, "%-4d %-4d %-7s %-13s %-6d %-12s %-11d %-11d %-11d %-11d %-8d %-8d %-8d", 
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

        mvprintw(i, 2, "%-4d %-4d %-7s %-13s %-6d %-12s %-11d %-11d %-11d %-11d %-8d %-8d %-8d", 
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

        char *estado_termino;
        if (aux_te->estadoTermino == 0){
            estado_termino = "terminados";
        } else if(aux_te->estadoTermino == 1) {
            estado_termino = "terminados*";
        } else if(aux_te->estadoTermino == 2) {
            estado_termino = "terminados**";
        }

        mvprintw(i, 2, "%-4d %-4d %-7s %-13s %-6d %-12s %-11d %-11d %-11d %-11d %-8d %-8d %-8d", 
        aux_te->PID,
        aux_te->GID,
        aux_te->archivo,
        estado_termino,
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

//Imprime el porcentaje de uso en RAM y en disco
void porcentajeDiscoRAM(TablaMarcos *tmm, TablaMarcos *tms){
    int ocupado_disco = 0;
    int ocupado_ram = 0;

    for (int i=0; i<TOTAL_MARCOS_RAM; i++){
        if(tmm[i].propietario != 0){
            ocupado_ram++;
        }
    }

    for (int i=0; i<TOTAL_MARCOS_DISCO; i++){
        if(tms[i].propietario != 0){
            ocupado_disco++;
        }
    }

    mvprintw(28, 180, "-%25s", " ");
    mvprintw(29, 180, "-%25s", " ");

    mvprintw(28, 173, "Uso RAM: %d / 16", ocupado_ram);
    mvprintw(29, 173, "Uso DISCO: %d / 32768", ocupado_disco);
}