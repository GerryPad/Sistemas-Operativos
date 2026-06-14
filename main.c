#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <math.h>
#include <time.h>
#include <sys/select.h>
#include "instrucciones.h"
#include "ncurses.h"
#include "nodo.h"
#include "dispatch.h"
#include "swap.h"
#include "ram.h"

#define TAMANO_IR 64 //El tamaño de instruccion
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) //256 bytes
#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768 //2ˆ15

FILE *bin;

TablaMarcos tmm[TOTAL_MARCOS_RAM];
TablaMarcos tms[TOTAL_MARCOS_DISCO];

char RAM[TOTAL_MARCOS_RAM*TAMANO_MARCO]; //64 intrucciones = 4096 bytes

int kbhit(void);        
int main(){

    //Cabeceras de las listas para los 5 estados posibles de procesos
    struct Nodo *nuevos = crearCabecera();
    struct Nodo *listos = crearCabecera();
    struct Nodo *terminados = crearCabecera();
    struct Nodo *ejecutando = crearCabecera();
    struct Nodo *suspendidos = crearCabecera();

    //Nodos auxiliares que apuntan a los diferentes procesos a crear, ejecutar, terminar, etc
    struct Nodo *nuevo = NULL;
    struct Nodo *proceso_actual = NULL; //El que se esta ejecutando
    struct Nodo *proceso_a_terminar = NULL;
    struct Nodo *proceso_a_matar = NULL;
    struct Nodo *proceso_a_copiar  = NULL;
    struct Nodo *proceso_a_suspender  = NULL;

    //Puntero (manecilla) que apunta incialmente al inicio de la TMM
    struct TablaMarcos *manecilla_reloj = &tmm[0];
    manecilla_reloj->puntero = true; //bandera para impresion de manecilla (*)

    //Buffers para nombre de archivo, tokenizacion de comandos e impresion a pantalla
    char archivo[64], linea[TAMANO_IR], comando[256], linea_original[128];
    int pc, com, pid=1, gid=1, pid_kill=0, num_inst = 0, quantum = 0; //num_inst actualiza el PC de un proceso hecho con fork
    int *ptr_pid = &pid_kill, *ptr_inst = &num_inst; //para modificar variables para comandos de 1 argumento o mas
    int total_instrucciones, total_marcos_necesarios, aux;
    int pag_actual, desplazamiento, marco_ram, pos_fisica;
    char *token, *ptr, *argumentos; //token almacena el nemonico, ptr quita los espacios y tabulaciones al inicio 
    bool tokEND, com_valido, interrumpido;
    bool fin_quantum, limpieza = false; 
    bool page_fault = false, speed = true;

    srand(time(NULL)); //Semilla para el tiempo de espera aletorio

    //Abrir/crear archivo binario en lectura y escritura
    bin = fopen("disco_virtual.bin", "wb+");
    if (!bin) {
        perror("fopen");
    }

    iniciarDiscoYTablas(tms, tmm, bin);
   
    initscr();
    do{
        tokEND = false;
        manecilla_reloj = sacarSuspendidos(suspendidos, listos, manecilla_reloj, ejecutando, tmm, tms, RAM, bin);
        sacarNuevos(nuevos,listos, tms, bin);
        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
        
        //Si no estamos ejecutando nada...
        if(ejecutando->siguiente == NULL){ 
            //Si las listas de listos O suspendidos tienen algo...
            if(listos->siguiente != NULL || suspendidos->siguiente != NULL){
                //Si hay al menos uno en listos -> planifica
                if(listos->siguiente !=NULL){
                    calculoPrioridades(listos, suspendidos, contarGrupos(listos, ejecutando, suspendidos, gid));
                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                    //if(!speed) usleep(3000000);
                    if(!speed) {
                        for(int i = 0; i < 60; i++) {
                            if(kbhit()) break; // Si tocas una tecla, rompe la pausa
                            usleep(50000);
                        }
                    }
                    proceso_actual = planificador(listos, ejecutando); 
                }
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                 
                if (proceso_actual != NULL) {
                    manecilla_reloj = sacarSuspendidos(suspendidos, listos, manecilla_reloj, ejecutando, tmm, tms, RAM, bin);
                    sacarNuevos(nuevos,listos, tms, bin);
                    pc = restauraPCB(proceso_actual, archivo); //Cargar contexto solo si se logro planificar
                } else { //Si todos los procesos estan suspendidos, hay que revisarlos constantemente
                    manecilla_reloj = sacarSuspendidos(suspendidos, listos, manecilla_reloj, ejecutando, tmm, tms, RAM, bin);
                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                    usleep(50000);
                    if(kbhit()){ 
                        if(limpieza){ 
                            limpia_lineas();
                        }

                        interrumpido=true;
                        refresh();
                        mvprintw(40, 2, "%-28s", ""); 
                        mvprintw(40, 2, ">");
                        echo();
                        memset(comando, 0, sizeof(comando));
                        //mvscanw(40,3,"%255[^\n]",comando);
                        
                        //Usar la lectura nativa y segura de ncurses para que no haya fallos aleatorios connn comandos validos
                        mvgetnstr(40, 3, comando, 255);
                        noecho();
                        limpieza = true;
                        com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst);

                        if (com == 1){
                            fclose(bin);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                if(verificarEspacioEnSwap(archivo, tms)){
                                    total_instrucciones = guardarTextoABinario(archivo, bin, pid, gid, tms);
                                    total_marcos_necesarios = ceil((float)total_instrucciones/INSTRUCCIONES_POR_MARCO);
                                    nuevo=crearNodo(pid, gid, archivo, total_marcos_necesarios);
                                    actualizaTMP(nuevo, tms);
                                    imprimirTms(tms);
                                    pid++;
                                    gid++;
                                    insertarFinal(listos,nuevo);
                                    interrumpido = false;
                                    continue;
                                } else{
                                        total_marcos_necesarios = cuentaMarcosNecesarios(archivo);
                                        if(total_marcos_necesarios > TOTAL_MARCOS_DISCO) {
                                            mvprintw(39, 2, "Este archivo excede la capacidad total del disco.");
                                        } else {
                                            nuevo = crearNodo(pid, gid, archivo, total_marcos_necesarios);
                                            pid++;
                                            gid++;
                                            insertarFinal(nuevos, nuevo);
                                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                            refresh();
                                        }
                                    }
                            } else {
                                mvprintw(37,2,"Archivo no existente");
                                limpieza = true;
                                mvprintw(40, 2, "%-28s", ""); 
                                refresh();
                            }      
                        
                        } else if(com == 3){
                            proceso_a_matar = extraerPID(ejecutando, pid_kill);
                            if(proceso_a_matar != NULL){
                                proceso_a_matar->estadoTermino = 2;
                                eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_matar);
                                porcentajeDiscoRAM(tmm, tms);
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            } else{
                                proceso_a_matar = extraerPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    proceso_a_matar->estadoTermino = 2;
                                    eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                    imprimirTmm(tmm);
                                    imprimirTms(tms);
                                    imprimirTmp(proceso_a_matar);
                                    porcentajeDiscoRAM(tmm, tms);
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } else {
                                    proceso_a_matar = extraerPID(suspendidos, pid_kill);
                                    if(proceso_a_matar != NULL){
                                        proceso_a_matar->estadoTermino = 2;
                                        eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                        imprimirTmm(tmm);
                                        imprimirTms(tms);
                                        imprimirTmp(proceso_a_matar);
                                        porcentajeDiscoRAM(tmm, tms);
                                        insertarFinal(terminados,proceso_a_matar);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    } else {
                                        proceso_a_matar = extraerPID(nuevos, pid_kill);
                                        if(proceso_a_matar != NULL){
                                            proceso_a_matar->estadoTermino = 2;
                                            eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                            imprimirTmm(tmm);
                                            imprimirTms(tms);
                                            imprimirTmp(proceso_a_matar);
                                            porcentajeDiscoRAM(tmm, tms);
                                            insertarFinal(terminados, proceso_a_matar);
                                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                        } else{
                                            mvprintw(37,2, "El PID asociado al proceso no existe o ya termino.");
                                        }
                                    }
                                    
                                }
                            } 
                        } else if(com ==4){
                            speed = !speed;
                        } else if(com == 5){
                            proceso_a_copiar = buscaPID(ejecutando, pid_kill);
                            if(proceso_a_copiar != NULL){
                                nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, proceso_a_copiar->num_paginas);
                                //Copiar la TMP del padre al hijo
                                for(int i = 0; i < proceso_a_copiar->num_paginas; i++){
                                    nuevo->tmp[i].num_marco_disco = proceso_a_copiar->tmp[i].num_marco_disco;
                                    nuevo->tmp[i].num_marco_ram   = proceso_a_copiar->tmp[i].num_marco_ram;
                                }
                                pid++;
                                nuevo->PC = num_inst;
                                nuevo->GCPU = proceso_a_copiar->GCPU;
                                insertarFinal(listos, nuevo);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            } else { 
                                proceso_a_copiar = buscaPID(listos, pid_kill);
                                if(proceso_a_copiar != NULL) {
                                    nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, proceso_a_copiar->num_paginas);
                                    for(int i = 0; i < proceso_a_copiar->num_paginas; i++){
                                        nuevo->tmp[i].num_marco_disco = proceso_a_copiar->tmp[i].num_marco_disco;
                                        nuevo->tmp[i].num_marco_ram   = proceso_a_copiar->tmp[i].num_marco_ram;
                                    }
                                    pid++;
                                    nuevo->PC = num_inst;
                                    nuevo->GCPU = proceso_a_copiar->GCPU;
                                    insertarFinal(listos, nuevo);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    
                                } else {
                                    proceso_a_copiar = buscaPID(suspendidos, pid_kill);
                                    if(proceso_a_copiar != NULL){    
                                        nuevo=crearNodo(pid,proceso_a_copiar->GID,proceso_a_copiar->archivo,proceso_a_copiar->num_paginas);
                                       for(int i = 0; i < proceso_a_copiar->num_paginas; i++){
                                            nuevo->tmp[i].num_marco_disco = proceso_a_copiar->tmp[i].num_marco_disco;
                                            nuevo->tmp[i].num_marco_ram   = proceso_a_copiar->tmp[i].num_marco_ram;
                                        }
                                        pid++;
                                        nuevo->PC = num_inst;
                                        nuevo->GCPU = proceso_a_copiar->GCPU;
                                        insertarFinal(listos,nuevo);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    }else{
                                        mvprintw(39,2,"No existe el proceso asociado al PID o el proceso ya termino.");
                                    }
                                }
                            }
                        } else {
                            if (com == -1) {
                                mvprintw(37,2, "Error: Falta nombre de archivo.");
                                limpieza = true;
                                continue;
                            } else {
                                mvprintw(37,2,"Error: Comando invalido");
                                limpieza = true;
                                continue;
                            }
                            refresh();
                            break;
                        }  
                    }
                }
            } else { //Peticion de comandos cuando no hay procesos activos
                com_valido = false;

                //Solicitamos comando hasta que haya uno valido
                while (!com_valido){ 
                    mvprintw(40, 2, "%-28s", ""); 
                    mvprintw(40,2, ">");
                    echo();
                    memset(comando, 0, sizeof(comando));
                    mvgetnstr(40, 3, comando, 255);
                    noecho();
                    mvprintw(40, 2, "%-28s", ""); 
                    refresh();
                
                    com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst); 
                    limpieza = true;

                    if (com == 1){ 
                        endwin();
                        fclose(bin);
                        return 0;
                    } else if (com == 2){ 
                        com_valido = true;
                        if(verificarEspacioEnSwap(archivo, tms)){
                            total_instrucciones = guardarTextoABinario(archivo, bin, pid, gid, tms);
                            total_marcos_necesarios = ceil((float)total_instrucciones/INSTRUCCIONES_POR_MARCO); 
                            nuevo=crearNodo(pid,gid,archivo, total_marcos_necesarios);
                            actualizaTMP(nuevo, tms);
                            pid++;
                            gid++;
                            insertarFinal(listos, nuevo);
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            refresh();
                            
                        } else {
                            total_marcos_necesarios = cuentaMarcosNecesarios(archivo);
                            if(total_marcos_necesarios > TOTAL_MARCOS_DISCO) {
                                mvprintw(39, 2, "Este archivo execde la capacidad total del disco.");
                            } else {
                                nuevo = crearNodo(pid, gid, archivo, total_marcos_necesarios);
                                pid++;
                                gid++;
                                insertarFinal(nuevos, nuevo);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                refresh();
                            }
                        }

                    } else if(com == 3){ 
                        mvprintw(37, 2, "No hay ningun proceso para matar.");
                    } else if (com == 4){ 
                        com_valido = true;
                        speed = !speed;
                    } else if (com == 5){ 
                        mvprintw(39, 0, "No hay procesos para copiar");
                    } else { 
                        if (com == -1) {
                            mvprintw(37,2, "Error: Comando incompleto.");
                        } else {
                            mvprintw(37,2,"Error: Comando invalido");
                        }
                        refresh();
                    }
                }
                continue;
            }
        }

        if(limpieza){
            limpia_lineas();
            limpieza = false;
        }

        //Ciclo de ejecucion
        if (proceso_actual != NULL) {
            quantum = 0;
            fin_quantum = false; 
            
            interrumpido=false; 
            strcpy(linea_original, proceso_actual->IR);
            manecilla_reloj = sacarSuspendidos(suspendidos, listos, manecilla_reloj, ejecutando, tmm, tms, RAM, bin);
            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
            //En vez de while(fgets), iteramos el tiempo de quantum
            while (quantum < 3) {
                memset(linea, 0, TAMANO_IR);
                pag_actual = pc / INSTRUCCIONES_POR_MARCO;
                if (pag_actual >= proceso_actual->num_paginas) {
                    mvprintw(36, 10, "Error: Se alcanzo el fin de memoria sin encontrar END o se intenta acceder a otra zona de memoria no permitida.");
                    tokEND = false; 
                    limpieza = true;
                    break;
                }
                
                desplazamiento = pc % INSTRUCCIONES_POR_MARCO;
                marco_ram = proceso_actual->tmp[pag_actual].num_marco_ram;
                pos_fisica = 0;
                
                //Fallo de pagina
                if (marco_ram == -1) {
                    //manecilla_reloj = algoritmoReloj(manecilla_reloj, listos, ejecutando, suspendidos, tmm, RAM);
                    page_fault = true;
                    proceso_a_suspender = desencolar(ejecutando);
                   
                    if (proceso_a_suspender != NULL) {    
                        insertarFinal(suspendidos, proceso_a_suspender);

                        //Asignacion de tiempo de espera aleatorio
                        proceso_a_suspender->hora_entrada = time(NULL);
                        proceso_a_suspender->tiempo_espera = 0;//rand() % (9) + 2; //%(9)+2

                        /*marco_ram_nuevo = cargarARAM(proceso_a_suspender->PID, proceso_a_suspender->GID, pag_actual, bin, manecilla_reloj, listos, ejecutando, suspendidos, tms, tmm, RAM);
                        actualizaTMP(proceso_a_suspender, tms);*/
                        guardaPCB(proceso_a_suspender,pc,linea_original);
                        
                        //manecilla_reloj->puntero = true;
                        //manecilla_reloj = manecilla_reloj->siguiente; //Nos movemos al siguiente marco del que acabamos de desalojar
                        //Actualizamos TMP del proceso para indicar que su pagina ya esta cargada en el marco n
                        //if (marco_ram_nuevo != -1) {
                        //    proceso_a_suspender->tmp[pag_actual].num_marco_ram = marco_ram_nuevo;
                        //    tmm[marco_ram_nuevo].usado_recien = 1;
                        //}
                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                        imprimirTmm(tmm);
                        porcentajeDiscoRAM(tmm, tms);
                        //tmm[marco_ram_nuevo].puntero=false;
                        refresh();
                    }
                    proceso_actual = NULL;
                    limpieza = true;
                    break;  //Sale del while de quantum
                }

                //Page hit
                if(marco_ram != -1) {
                    //Traduccion de direcciones
                    pos_fisica = (marco_ram * TAMANO_MARCO) + (desplazamiento * TAMANO_IR);
                    tmm[marco_ram].usado_recien = 1;
                    memcpy(linea, &RAM[pos_fisica], TAMANO_IR); //Lectura desde RAM
                    linea[TAMANO_IR] = '\0';
                }

                strcpy(linea_original, linea); //Para imprimir la linea original en PCB 
                imprimir_registros(pc, linea);
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                imprimirTmm(tmm);
                imprimirTmp(proceso_actual);
                imprimirTms(tms);
                refresh();

                *ptr_pid = 0;
                ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++; 
                token = strtok(ptr, " \r\n\t");

                if (tokEND){ //Si hayamos un END...
                    if (token != NULL) { //Pero hay mas cosas despues
                        mvprintw(36, 10, "Error: Contenido tras END en Renglon %d", pc);
                        proceso_a_terminar = desencolar(ejecutando); //Siguiendo la logica de Pedro

                        if (proceso_a_terminar != NULL) {
                            proceso_a_terminar->estadoTermino = 1;
                            eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                            imprimirTmm(tmm);
                            imprimirTms(tms);
                            imprimirTmp(proceso_a_terminar);
                            porcentajeDiscoRAM(tmm, tms);
                            insertarFinal(terminados, proceso_a_terminar);
                        }
                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                        tokEND = false; 
                        limpieza = true;
                        break;
                    }
                    pc++;
                    continue;
                }

                //Para aquellos renglones vacios
                if (token == NULL) {
                    pc++;
                    continue;
                }

                if (token!= NULL && validarToken(instruccion, token)){
                    argumentos = ptr + strlen(token) + 1; //Reconocer lo que esta despues del nemonico

                    if (strcmp(token, "END") == 0) {
                        char *extra = strtok(NULL, " \r\n\t"); 
                        
                        if (extra != NULL) {
                            //CASO 1: Hay basura despues del END
                            mvprintw(36, 10, "Error: Contenido tras END en Renglon %d", pc);
                            tokEND = false; 
                            limpieza = true;
                            break; // Salimos y el código de afuera lo manda a estadoTermino = 1
                        } else {
                            //CASO 2: Es un END limpio
                            if (instEND()) {
                                tokEND = true; 
                                limpieza = true;
                                break; // Salimos y el codigo de afuera lo manda a estadoTermino = 0
                            } else {
                                tokEND = false;
                                limpieza = true;
                                break;
                            }
                        }
                    } else if(strcmp(token, "JNZ") == 0){
                        aux = instJNZ(argumentos,proceso_actual, pc);
                        if(aux != -1){
                            pc = aux;
                            quantum++;
                            proceso_actual->CPU = proceso_actual->CPU + 20;
                            proceso_actual->GCPU=proceso_actual->GCPU + 20;
                            aumentaGCPU(listos, proceso_actual->GID);
                            aumentaGCPU(suspendidos, proceso_actual->GID);

                            if (quantum == 3) {
                                guardaPCB(proceso_actual, pc, linea_original);
                                proceso_a_terminar = desencolar(ejecutando);
                                if (proceso_a_terminar!= NULL) {
                                    proceso_a_terminar->estadoTermino = 0;
                                    insertarFinal(listos, proceso_a_terminar);
                                }
                                proceso_actual = NULL;
                                fin_quantum = true;
                                limpieza = true;
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                refresh();
                                break; 
                            }
                            continue;
                        } else {
                            guardaPCB(proceso_actual,pc,linea_original);
                            mvprintw(34, 2, "ABORTADO: Error en renglon %d", pc);
                            mvprintw(36,2, "Motivo:");
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                proceso_a_terminar->estadoTermino = 1;
                                eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_terminar);
                                porcentajeDiscoRAM(tmm, tms);
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);

                            limpieza = true;
                            break; 
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos, proceso_actual, ptr_pid)) {
                            guardaPCB(proceso_actual,pc,linea_original);
                            mvprintw(34, 2, "ABORTADO: Error en renglon %d", pc);
                            mvprintw(36,2, "Motivo:");
                            //Mover los procesos fallidos a terminados
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                proceso_a_terminar->estadoTermino = 1;
                                eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_terminar);
                                porcentajeDiscoRAM(tmm, tms);
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            limpieza = true;
                            break; 
                        }
                    }
                    
                    if(*ptr_pid != -1){
                        pc++;
                    }

                    quantum++;
                    proceso_actual->CPU = proceso_actual->CPU + 20;
                    proceso_actual->GCPU=proceso_actual->GCPU + 20;
                    aumentaGCPU(listos,proceso_actual->GID);
                    aumentaGCPU(suspendidos,proceso_actual->GID);
                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                   
                    /*if(!speed){
                        usleep(500000);
                    }*/
                   
                    if(!speed) {
                        for(int i = 0; i < 10; i++) {
                            if(kbhit()) break; // Si tocas una tecla, despierta
                            usleep(50000);
                        }
                    }

                    if(tokEND){
                        continue;
                    }

                    if (quantum == 3) {
                        guardaPCB(proceso_actual, pc, linea_original);
                        proceso_a_terminar = desencolar(ejecutando);
                        if (proceso_a_terminar!= NULL) {
                            proceso_a_terminar->estadoTermino = 0;
                            insertarFinal(listos, proceso_a_terminar);
                        }
                        proceso_actual = NULL;
                        fin_quantum = true;
                        limpieza = true;
                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                        refresh();  
                        break; 
                    }

                    if(kbhit()){ 
                        if(limpieza){ 
                            limpia_lineas();
                        }
                        interrumpido=true;
                        refresh();
                        mvprintw(40, 2, "%-28s", ""); 
                        mvprintw(40, 2, ">");
                        echo();
                        memset(comando, 0, sizeof(comando));
                        //mvscanw(40,3,"%255[^\n]",comando)
                        mvgetnstr(40, 3, comando, 255);
                        noecho();
                        limpieza = true;
                        com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst);

                        if (com == 1){
                            fclose(bin);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                if(verificarEspacioEnSwap(archivo, tms)){
                                    total_instrucciones = guardarTextoABinario(archivo, bin, pid, gid, tms);
                                    total_marcos_necesarios = ceil((float)total_instrucciones/INSTRUCCIONES_POR_MARCO);
                                    nuevo=crearNodo(pid, gid, archivo, total_marcos_necesarios);
                                    actualizaTMP(nuevo, tms);
                                    imprimirTms(tms);
                                    pid++;
                                    gid++;
                                    insertarFinal(listos,nuevo);
                                    interrumpido = false;
                                    continue; //Para seguir con el proceso actual y que no se cambie por el nuevo
                                } else{
                                        total_marcos_necesarios = cuentaMarcosNecesarios(archivo);
                                        if(total_marcos_necesarios > TOTAL_MARCOS_DISCO) {
                                            mvprintw(39, 2, "Este archivo execde la capacidad total del disco.");
                                        } else {
                                            nuevo = crearNodo(pid, gid, archivo, total_marcos_necesarios);
                                            pid++;
                                            gid++;
                                            insertarFinal(nuevos, nuevo);
                                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                            refresh();
                                        }
                                    }
                            } else {
                                mvprintw(37,2,"Archivo no existente");
                                limpieza = true;
                                mvprintw(40, 2, "%-28s", ""); 
                                refresh();
                            }      
                        
                        } else if(com == 3){
                            proceso_a_matar = extraerPID(ejecutando, pid_kill);
                            if(proceso_a_matar != NULL){
                                proceso_a_matar->estadoTermino = 2;
                                eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_matar);
                                porcentajeDiscoRAM(tmm, tms);
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                break; 
                            } else{
                                proceso_a_matar = extraerPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    proceso_a_matar->estadoTermino = 2;
                                    eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                    imprimirTmm(tmm);
                                    imprimirTms(tms);
                                    imprimirTmp(proceso_a_matar);
                                    porcentajeDiscoRAM(tmm, tms);
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } else {
                                    proceso_a_matar = extraerPID(suspendidos, pid_kill);
                                    if(proceso_a_matar != NULL){
                                        proceso_a_matar->estadoTermino = 2;
                                        eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                        imprimirTmm(tmm);
                                        imprimirTms(tms);
                                        imprimirTmp(proceso_a_matar);
                                        porcentajeDiscoRAM(tmm, tms);
                                        insertarFinal(terminados, proceso_a_matar);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    } else {
                                        proceso_a_matar = extraerPID(nuevos, pid_kill);
                                        if(proceso_a_matar != NULL){
                                            proceso_a_matar->estadoTermino = 2;
                                            eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                                            imprimirTmm(tmm);
                                            imprimirTms(tms);
                                            imprimirTmp(proceso_a_matar);
                                            porcentajeDiscoRAM(tmm, tms);
                                            insertarFinal(terminados, proceso_a_matar);
                                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                        } else{
                                            mvprintw(37,2, "El PID asociado al proceso no existe o ya termino.");
                                        }
                                    }
                                }
                            } 
                        } else if(com == 4){ //Ahora es la logica del comando fast
                            speed = !speed;
                        } else if(com == 5){
                            proceso_a_copiar = buscaPID(ejecutando, pid_kill);
                            if(proceso_a_copiar != NULL){
                                
                                nuevo=crearNodo(pid,proceso_a_copiar->GID,proceso_a_copiar->archivo,proceso_a_copiar->num_paginas);
                                for(int i = 0; i < proceso_a_copiar->num_paginas; i++){
                                    nuevo->tmp[i].num_marco_disco = proceso_a_copiar->tmp[i].num_marco_disco;
                                    nuevo->tmp[i].num_marco_ram   = proceso_a_copiar->tmp[i].num_marco_ram;
                                }
                                pid++;
                                nuevo->PC = num_inst;
                                nuevo->GCPU = proceso_a_copiar->GCPU;
                                insertarFinal(listos, nuevo);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                 //creo que aqui falta mandarlo a nuevos 
                              
                            } else { 
                                proceso_a_copiar = buscaPID(listos, pid_kill);
                                if(proceso_a_copiar != NULL) {
                                    
                                        nuevo=crearNodo(pid,proceso_a_copiar->GID,proceso_a_copiar->archivo,proceso_a_copiar->num_paginas);
                                        //nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, total_marcos_necesarios);
                                        for(int i = 0; i < proceso_a_copiar->num_paginas; i++){
                                            nuevo->tmp[i].num_marco_disco = proceso_a_copiar->tmp[i].num_marco_disco;
                                            nuevo->tmp[i].num_marco_ram   = proceso_a_copiar->tmp[i].num_marco_ram;
                                        }
                                        pid++;
                                        nuevo->PC = num_inst;
                                        nuevo->GCPU = proceso_a_copiar->GCPU;
                                        insertarFinal(listos, nuevo);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    
                                } else {
                                    proceso_a_copiar = buscaPID(suspendidos, pid_kill);
                                    if(proceso_a_copiar != NULL){
                                        nuevo=crearNodo(pid,proceso_a_copiar->GID,proceso_a_copiar->archivo,proceso_a_copiar->num_paginas);
                                        for(int i = 0; i < proceso_a_copiar->num_paginas; i++){
                                            nuevo->tmp[i].num_marco_disco = proceso_a_copiar->tmp[i].num_marco_disco;
                                            nuevo->tmp[i].num_marco_ram   = proceso_a_copiar->tmp[i].num_marco_ram;
                                        }
                                        pid++;
                                        nuevo->PC = num_inst;
                                        nuevo->GCPU = proceso_a_copiar->GCPU;
                                        insertarFinal(listos,nuevo);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                        
                                    }else{
                                        mvprintw(39,2,"No existe el proceso asociado al PID o el proceso ya termino.");
                                    }
                                }
                            }
                        } else {
                            if (com == -1) {
                                mvprintw(37,2, "Error: Falta nombre de archivo.");
                                limpieza = true;
                                continue;
                            } else {
                                mvprintw(37,2,"Error: Comando invalido");
                                limpieza = true;
                                continue;
                            }
                            refresh();
                            break;
                        }   
                    }
                } else {
                    mvprintw(36, 2, "Token no valido: [%s]", token);
                    proceso_a_terminar = desencolar(ejecutando);
                    if (proceso_a_terminar != NULL) {
                        proceso_a_terminar->estadoTermino = 1;
                        eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                        imprimirTmm(tmm);
                        imprimirTms(tms);
                        imprimirTmp(proceso_a_terminar);
                        porcentajeDiscoRAM(tmm, tms);
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    limpieza = true;
                    break;
                }               
            }

            manecilla_reloj = sacarSuspendidos(suspendidos, listos, manecilla_reloj, ejecutando, tmm, tms, RAM, bin);
            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
            if(page_fault){
                page_fault = false;
            } else if(fin_quantum){ //esta bandera evita el doble cierre de archivos y el core dumpesd
                mvprintw(35, 2, "Quantum = 3. Cambio de proceso");
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                refresh();
            } else if(!interrumpido){ //Cuando el quantum no termina, osea no es multiplo de 3 el numero de instrucciones
                if (tokEND){
                    mvprintw(35, 2, "Estado: Procesado con éxito.");
                    if(proceso_actual!=NULL) {
                        guardaPCB(proceso_actual,pc,linea_original);
                        proceso_a_terminar = desencolar(ejecutando);
                        if (proceso_a_terminar != NULL) {
                            proceso_a_terminar->estadoTermino = 0;
                            eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                            imprimirTmm(tmm);
                            imprimirTms(tms);
                            imprimirTmp(proceso_a_terminar);
                            porcentajeDiscoRAM(tmm, tms);
                            insertarFinal(terminados, proceso_a_terminar);
                        }
                    }
                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                } else {
                    mvprintw(35, 2, "Estado: Error - Falto END o abortado.");
                    limpieza = true;
                    if(proceso_actual!=NULL) {
                        guardaPCB(proceso_actual,pc,linea_original);
                        proceso_a_terminar = desencolar(ejecutando);
                        if (proceso_a_terminar != NULL) {
                            proceso_a_terminar->estadoTermino = 1;
                            eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                            imprimirTmm(tmm);
                            imprimirTms(tms);
                            imprimirTmp(proceso_a_terminar);
                            porcentajeDiscoRAM(tmm, tms);
                            insertarFinal(terminados, proceso_a_terminar);
                        }
                    }
                    proceso_actual = NULL;
                }
                proceso_actual = NULL;
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                refresh();
            } else { //este era el else de cuando se ejecutaba el archivo hasta el final
                if(proceso_actual!=NULL){
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);
                    if (proceso_a_terminar != NULL) {
                        proceso_a_terminar->estadoTermino = 0;
                        eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos, bin, RAM);
                        imprimirTmm(tmm);
                        imprimirTms(tms);
                        imprimirTmp(proceso_a_terminar);
                        porcentajeDiscoRAM(tmm, tms);
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                }
                proceso_actual = NULL;
            }
        } else {
            manecilla_reloj = sacarSuspendidos(suspendidos, listos, manecilla_reloj, ejecutando, tmm, tms, RAM, bin);
            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
            limpieza = true;   
        }
    } while(1);
    endwin();
    return 0;
}

int kbhit(void) 
{
    struct timeval tv;
    fd_set read_fd;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&read_fd);
    FD_SET(0, &read_fd);

    if (select(1, &read_fd, NULL, NULL, &tv) == -1)
        return 0;

    if (FD_ISSET(0, &read_fd))
        return 1;

    return 0;
}