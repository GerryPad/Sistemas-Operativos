#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"
#include "ncurses.h"
#include "nodo.h"
#include "dispatch.h"
#include <sys/select.h>

struct Nodo *buscaPID(struct Nodo *lista, int pid){
    struct Nodo * aux = lista->siguiente;

    while(aux != NULL && aux->PID != pid){
        aux = aux->siguiente;
    
    }

    if(aux==NULL){
        return NULL;
    }
    return aux;
}

struct Nodo *buscaGID(struct Nodo *lista, int gid) {
    struct Nodo *aux = lista->siguiente;

    while (aux != NULL && aux->GID != gid) {
        aux = aux->siguiente;
    }

    if(aux==NULL){
            return NULL;
        }
        //aux->GCPU= aux->GCPU+20;
        return aux;
}

void aumentaGCPU(struct Nodo *listos, int gid){
    struct Nodo *aux = listos->siguiente;

    while (aux != NULL) {
        if(aux->GID== gid){
            aux->GCPU= aux->GCPU+20;
        }
        aux = aux->siguiente;
    }
    

}

int contarGrupos(struct Nodo *listos, struct Nodo *ejecutando, int max_gid) {
    int contador = max_gid;  //Asumir que todos los grupos estan activos

    for (int i = 1; i <= max_gid; i++) {
        if (buscaGID(ejecutando, i) != NULL) {
            continue; //Si esta en ejecutando ya no lo busques en listos
        } else {
            if (buscaGID(listos, i) == NULL) {
                //Si tampoco esta en listos, el grupo no esta activo
                contador--;
            }
        }
    }

    mvprintw(30, 0, "Grupos: %-4d", contador);
    return contador;
}

void calculoPrioridades(struct Nodo *listos, int wk, struct Nodo *saliendo) {
    
    // Solo castigamos/aliviamos al que acaba de usar la CPU
    saliendo->CPU /= 2;
    saliendo->GCPU /= 2;
    
    // Recalculamos prioridades de todos sin dividir sus CPUs entre 2
    struct Nodo *aux = listos->siguiente;
    while(aux != NULL) {
        aux->prioridad = 20 + (aux->CPU / 2) + (aux->GCPU / (4 * wk));
        aux = aux->siguiente;
    }

    /*struct Nodo *aux = listos;
    int p_base = 20;

    while(aux!= NULL){
        aux->CPU = aux->CPU/2;
        aux->GCPU = aux->GCPU/2;
        aux->prioridad = p_base + (aux->CPU/2) + (aux->GCPU/4*wk);
        aux = aux->siguiente;
    }*/
}

int kbhit(void);        
int main(){

    //Creando nodo de prueba para impresion
    struct Nodo *listos = crearCabecera();
    struct Nodo *terminados = crearCabecera();
    struct Nodo *ejecutando = crearCabecera();
    struct Nodo *nuevo;
    struct Nodo *proceso_actual = NULL; //El que se esta ejecutando
    struct Nodo *proceso_a_terminar = NULL;
    struct Nodo *proceso_a_matar = NULL;
    struct Nodo *proceso_a_copiar  = NULL;


    char archivo[64], linea[128], comando[256], com_mata[256], linea_original[128];; //Buffers para leer nombre y linea del archivo.
    int pc, com, pid=1, gid=1, pid_kill=0, num_inst = 0, quantum = 0, wk = 0, *ptr_pid = &pid_kill, *ptr_inst = &num_inst; //com es para hacer un "switch" 
    char *token, *ptr, *argumentos;
    bool tokEND, com_valido, interrumpido; //com_valido es para comprobar la existencia del comando
    bool fin_quantum, limpieza = false; 
    
    initscr();
    do{
        tokEND = false;

        if(ejecutando->siguiente == NULL){ //Cambiar el uso de la bandera pedir archivo
            if(listos->siguiente != NULL){
                proceso_actual = planificador(listos, ejecutando); //Hacer que el planificador te de el primero de listos

                //cargar su "contexto", de momento pues esta en ceros
                pc = restauraPCB(proceso_actual, archivo); 

            } else {
                com_valido = false; //Suponemos de entrada que el comando no es valido

                while (!com_valido){ //Solicitamos comando hasta que haya uno valido
                
                    move(20,2);
                    clrtoeol();
                    mvprintw(20,2, ">");
                    echo();
                    getstr(comando);
                    noecho();
                    move(20, 2); 
                    clrtoeol(); 
                    refresh();
                
                    com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst); 
                    limpieza = true;

                    if (com == 1){ //comando salir
                        endwin();
                        return 0;
                    } else if (com == 2){ //comando ejecuta
                        com_valido = true;
                        nuevo=crearNodo(pid, gid, archivo);
                        pid++;
                        gid++;
                        insertarFinal(listos,nuevo);
                    } else if(com == 3){ //comando mata
                        mvprintw(25, 2, "No hay ningun proceso para matar.");
                    } else if (com == 4){ //comando prueba
                        com_valido = true;
                        nuevo=crearNodo(pid, gid, "file"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file2"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file3"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file4"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file5"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file6"); pid++; gid++; insertarFinal(listos,nuevo);
                    } else if (com == 5){ //comando fork
                        mvprintw(30, 0, "Si entra al comando fork");
                    }
                    
                    else { //error al ingresar comando
                        move(25,2);
                        clrtoeol();
                        if (com == -1) {
                            mvprintw(25,2, "Error: Falta nombre de archivo.");
                        } else {
                            mvprintw(25,2,"Error: Comando invalido");
                            
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

        if (access(archivo, F_OK) == 0) {
            FILE *file = fopen(archivo, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }

            quantum = 0;
            fin_quantum = false; //para saber porque motivo cerramos proceso
            contarGrupos(listos, ejecutando, gid);
            
            int i=1;
            while(i<pc && fgets(linea, sizeof(linea), file) != NULL){
                i++;
                continue;
            }
           
            interrumpido=false; //Bandera para cada archivo
            strcpy(linea_original, "---");
            while (fgets(linea, sizeof(linea), file) != NULL) {
                linea[strcspn(linea, "\n\r")] = '\0';
                strcpy(linea_original, linea);//Para imprimir la linea original en PCB
                imprimir_registros(pc, linea);
                imprimir_listas(ejecutando, listos, terminados);
                refresh();
                
                ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++; 
                token = strtok(ptr, " \n");

                if (tokEND){ //Si hayamos un END...
                    if (token != NULL) { //Pero hay mas cosas despues
                        move(24,10);
                        clrtoeol();
                        mvprintw(24, 10, "Error: Contenido tras END en Renglon %d", pc);
                        proceso_a_terminar = desencolar(ejecutando); //Siguiendo la logica de Pedro

                        if (proceso_a_terminar != NULL) {
                            strcpy(proceso_a_terminar->estado, "terminado*");
                            insertarFinal(terminados, proceso_a_terminar);
                        }
                        imprimir_listas(ejecutando, listos, terminados);
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

                    if (strcmp(token, "END") == 0){
                        if (instEND()) {
                            tokEND = true;
                        } else {
                            tokEND = false;
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                strcpy(proceso_a_terminar->estado, "terminado*");
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            limpieza=true;
                            break;
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos)) {
                            guardaPCB(proceso_actual,pc,linea_original);
                            mvprintw(22, 2, "ABORTADO: Error en renglon %d", pc);
                            mvprintw(24,2, "Motivo:");
                            //Mover los procesos fallidos a terminados
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                strcpy(proceso_a_terminar->estado, "terminado*");
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            imprimir_listas(ejecutando, listos, terminados);

                            limpieza = true;
                            break; 
                        }
                    }
                    usleep(1000000);
                    pc++;
                    quantum++;
                    proceso_actual->CPU = proceso_actual->CPU + 20;
                    proceso_actual->GCPU=proceso_actual->GCPU + 20;
                    aumentaGCPU(listos,proceso_actual->GID);
                    if(tokEND){
                        continue;
                    }

                    if (quantum == 3) {
                        guardaPCB(proceso_actual, pc, linea_original);
                        proceso_a_terminar = desencolar(ejecutando);
                        if (proceso_a_terminar!= NULL) {
                            strcpy(proceso_a_terminar->estado, "listos");
                            insertarFinal(listos, proceso_a_terminar);
                        }
                        fclose(file);
                        proceso_actual = NULL;
                        fin_quantum = true;
                        limpieza = true;
                        imprimir_listas(ejecutando, listos, terminados);
                        refresh();
                        
                        break; 
                    }

                    //Movi el kbhit despues para que no haga falta aumentar pc dentro de kbhit
                    if(kbhit()){ //Cuando haya un teclazo
                        if(limpieza){ //tambien puede que no sea correco guardarlo asi
                            limpia_lineas();
                        }
                        interrumpido=true;
                        getch();

                        refresh();
                        move(20,2);
                        clrtoeol();
                        mvprintw(20, 2, ">");
                        echo();
                        mvscanw(20,3,"%255[^\n]",comando);
                        noecho();
                        limpieza = true;
                        strcpy(com_mata, comando);
                        com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst);

                        if (com == 1){
                            fclose(file);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                nuevo=crearNodo(pid, gid, archivo);
                                pid++;
                                gid++;
                                insertarFinal(listos,nuevo);
                                interrumpido = false;
                                continue; //Para seguir con el proceso actual y que no se cambie por el nuevo
                            } else {
                                move(25,2);
                                clrtoeol();
                                mvprintw(25,2,"Archivo no existente");
                                limpieza = true;
                                move(20,2);
                                clrtoeol();
                                refresh();
                            }      
                        
                        } else if(com == 3){
                            proceso_a_matar = mataPID(ejecutando, pid_kill);
                            if(proceso_a_matar != NULL){
                                strcpy(proceso_a_matar->estado, "terminados**");
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando,listos,terminados);
                                break; 
                            } else{
                                proceso_a_matar = mataPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    strcpy(proceso_a_matar->estado, "terminados**");
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando,listos,terminados);
                                } else {
                                    move(25,2);
                                    clrtoeol();
                                    mvprintw(25,2, "El PID asociado al proceso no existe.");
                                    mvprintw(27,2, "Ese proceso no existe o ya termino");
                                }
                            } 
                        } else if(com == 5){
                            proceso_a_copiar = buscaPID(ejecutando, pid_kill);
                            if(proceso_a_copiar != NULL){
                               nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo);
                               pid++;
                               nuevo->PC = num_inst;
                               insertarFinal(listos, nuevo);
                               imprimir_listas(ejecutando, listos, terminados);
                            } else {
                                proceso_a_copiar = buscaPID(listos, pid_kill);
                                if(proceso_a_copiar != NULL) {
                                    nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo);
                                    pid++;
                                    nuevo->PC = num_inst;
                                    insertarFinal(listos, nuevo);
                                    imprimir_listas(ejecutando, listos, terminados);
                                } else {
                                    move(30,2);
                                    clrtoeol();
                                    mvprintw(30,2,"No existe el proceso asociado al PID o el proceso ya termino.");
                                }
                                
                            }

                        }
                        
                        else {
                            move(25,2);
                            clrtoeol();
                            if (com == -1) {
                                mvprintw(25,2, "Error: Falta nombre de archivo.");
                                limpieza = true;
                                continue;
                            } else {
                                mvprintw(25,2,"Error: Comando invalido");
                                limpieza = true;
                                continue;
                            }
                            refresh();
                            fclose(file);
                            break;
                        }
                         
                    }

                } else {
                    move(24,2);
                    clrtoeol();
                    mvprintw(24, 2, "Token no valido: [%s]", token);
                    proceso_a_terminar = desencolar(ejecutando);
                    if (proceso_a_terminar != NULL) {
                        strcpy(proceso_a_terminar->estado, "terminado*");
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    limpieza = true;
                    break;
                }               
            }
            
            if(fin_quantum){ //esta bandera evita el doble cierre de archivos y el core dumpesd
                move(23, 2); clrtoeol();
                mvprintw(23, 2, "Quantum = 3. Cambio de proceso");
                calculoPrioridades(listos,contarGrupos(listos,ejecutando,gid),proceso_a_terminar); //agregamos el nodo que solo queremos castigar
                refresh();
            } else if(!interrumpido){
                move(23, 2); clrtoeol();
                if (tokEND){
                    move(23,2);
                    clrtoeol();
                    mvprintw(23, 2, "Estado: Procesado con éxito.");
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);

                    if (proceso_a_terminar != NULL) {
                        strcpy(proceso_a_terminar->estado, "terminado");
                        insertarFinal(terminados, proceso_a_terminar);
                    }

                    imprimir_listas(ejecutando, listos, terminados);
                } else {
                    mvprintw(23, 2, "Estado: Error - Falto END o abortado.");
                    limpieza = true;
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);

                    if (proceso_a_terminar != NULL) {
                        strcpy(proceso_a_terminar->estado, "terminado*");
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    proceso_actual = NULL;
                }
                fclose(file);
                proceso_actual = NULL;
                imprimir_listas(ejecutando, listos, terminados);
                refresh();
            } else {
                fclose(file);
                if(proceso_actual!=NULL){
                    guardaPCB(proceso_actual,pc,linea_original);
                }

                proceso_a_terminar = desencolar(ejecutando);
                if (proceso_a_terminar != NULL) {
                    strcpy(proceso_a_terminar->estado, "terminado");
                    insertarFinal(terminados, proceso_a_terminar);
                }
                //Si el proceso actual termino, ya no estamos ejecutando nada
                proceso_actual = NULL;
            }

        } else {
            mvprintw(22, 2, "El archivo NO existe.");
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