#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <math.h>
#include "instrucciones.h"
#include "ncurses.h"
#include "nodo.h"
#include "dispatch.h"
#include <sys/select.h>

#define TAMANO_IR 64
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) 
#define TOTAL_MARCOS_RAM 16

typedef struct {
    int num_marco;
    int propietario; // 0= libre, 0 != pid asignado
} TMM; 

TMM tmm[TOTAL_MARCOS_RAM];


char RAM[TOTAL_MARCOS_RAM]; //1 = libre, 0 = ocupado
//char RAM[64][TAMANO_IR] //Esto es una forma de hacerlo, no la unica char RAM[64*TAMANO_IR]

void guardarTextoABinario(const char *archivoTexto, const char *archivoBinario) {
    FILE *txt = fopen(archivoTexto, "r");
    FILE *bin = fopen(archivoBinario, "wb"); 

    if (!txt || !bin) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    char linea[64];
    char bufferFijo[TAMANO_IR];

    // Leer el archivo de texto línea por línea
    while (fgets(linea, sizeof(linea), txt)) {
        linea[strcspn(linea, "\r\n")] = 0;

        //Saltar líneas vacías
        if (strlen(linea) == 0) continue;

        //Llenamos el marco inicialmente con 0's
        memset(bufferFijo, 0, TAMANO_IR);

        //Copiamos el texto de la instrucción al buffer seguro
        strncpy(bufferFijo, linea, TAMANO_IR - 1);

        //Escribimos exactamente 64 bytes en el archivo binario
        fwrite(bufferFijo, sizeof(char), TAMANO_IR, bin);
    }

    fclose(txt);
    fclose(bin);
}

int kbhit(void);        
int main(){

    //Creando nodo de prueba para impresion
    struct Nodo *nuevos = crearCabecera();
    struct Nodo *listos = crearCabecera();
    struct Nodo *terminados = crearCabecera();
    struct Nodo *ejecutando = crearCabecera();
    struct Nodo *suspendidos = crearCabecera();
    struct Nodo *nuevo;
    struct Nodo *proceso_actual = NULL; //El que se esta ejecutando
    struct Nodo *proceso_a_terminar = NULL;
    struct Nodo *proceso_a_matar = NULL;
    struct Nodo *proceso_a_copiar  = NULL;

    char archivo[64], linea[128], comando[256], linea_original[128];//, com_mata[256]; //Buffers para leer nombre y linea del archivo.
    int pc, com, pid=1, gid=1, pid_kill=0, num_inst = 0, quantum = 0;
    int *ptr_pid = &pid_kill, *ptr_inst = &num_inst, *ptr_pc=&pc; //com es para hacer un "switch" 
    int total_instrucciones, total_marcos_necesarios, cnt_marcos_libres;
    char *token, *ptr, *argumentos;
    bool tokEND, com_valido, interrumpido; //com_valido es para comprobar la existencia del comando
    bool fin_quantum, limpieza = false; 
    long tamano_bytes;

    /*for (int i=0; i<TOTAL_MARCOS_RAM; i++){
        RAM[i] = true;
    }*/

    for (int i=0; i<TOTAL_MARCOS_RAM; i++) {
        tmm[i].num_marco = i;
        tmm[i].propietario = 0;
    }
    
    initscr();
    do{
        tokEND = false;

        if(ejecutando->siguiente == NULL){ //Cambiar el uso de la bandera pedir archivo
            if(listos->siguiente != NULL){
                calculoPrioridades(listos,contarGrupos(listos,ejecutando,gid));
                actualizaCGPU(suspendidos->siguiente);
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                //usleep(3000000);
                proceso_actual = planificador(listos, ejecutando); //Hacer que el planificador te de el primero de listos

                //cargar su "contexto", de momento pues esta en ceros
                pc = restauraPCB(proceso_actual, archivo); 

            } else {
                com_valido = false; //Suponemos de entrada que el comando no es valido

                while (!com_valido){ //Solicitamos comando hasta que haya uno valido
                
                    move(40,2);
                    clrtoeol();
                    mvprintw(40,2, ">");
                    echo();
                    comando[0] = '\0';
                    getstr(comando);
                    noecho();
                    move(40, 2); 
                    clrtoeol(); 
                    refresh();
                
                    com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst); 
                    limpieza = true;

                    if (com == 1){ //comando salir
                        endwin();
                        return 0;
                    } else if (com == 2){ //comando ejecuta
                        com_valido = true;
                        guardarTextoABinario(archivo, "disco_virtual.bin");

                        FILE *bin = fopen("disco_virtual.bin", "rb");
                        if (!bin) {
                            perror("fopen");
                            continue;
                        }
                        fseek(bin, 0, SEEK_END); //Movernos al final del archivo
                        tamano_bytes = ftell(bin); //ftell devuelve en bytes la posicion actual del archivo, equivalente a la cantidad
                        fclose(bin);
                        total_instrucciones = tamano_bytes/TAMANO_IR; //En teoria deberia ser forzosamente un entero
                        total_marcos_necesarios = ceil(total_instrucciones/INSTRUCCIONES_POR_MARCO); 

                        cnt_marcos_libres = 0;
                        for(int m = 0; m < TOTAL_MARCOS_RAM; m++) {
                            if(tmm[m].propietario == 0){
                                cnt_marcos_libres++;
                            } 
                        }

                        if (cnt_marcos_libres < total_marcos_necesarios) {  //FAltaria la logica de swapping
                            nuevo=crearNodo(pid,gid,archivo);
                            pid++;
                            gid++;
                            insertarFinal(nuevos, nuevo);
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            mvprintw(37, 2, "Error: Memoria RAM insuficiente para el proceso."); //Quiatre esto cuando ya tenga la logica
                            refresh();
                            continue;
                        } else { //Aqui iria la logica de fallo de pagina
                            bin = fopen("disco_virtual.bin", "rb");
                            if (!bin) {
                                perror("fopen");
                                continue;
                            }
                            int pagina_actual = 0;
                            for(int i=0; i<TOTAL_MARCOS_RAM; i++){
                                if(tmm[i].propietario == 0){
                                    fseek(bin, pagina_actual*TAMANO_MARCO, SEEK_SET);

                                    if(fread(RAM + (i*TAMANO_MARCO), sizeof(char), TAMANO_IR, bin) > 0) {
                                        tmm[i].propietario = pid;
                                        pagina_actual++;
                                    } else {
                                        break;
                                    }
                                }
                            }
                            //Modifciar el abrir/cerrar archivos solo una vez con el bin
                            fclose(bin);
                            nuevo=crearNodo(pid, gid, archivo);
                            pid++;
                            gid++;
                            insertarFinal(suspendidos,nuevo); //Debe quedarse aqui un ratito aleatorio
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            usleep(5000000);
                            /*nuevo = extraerPID(suspendidos, pid-1);
                            insertarFinal(listos, nuevo);*/
                        }

                    } else if(com == 3){ //comando mata
                        mvprintw(37, 2, "No hay ningun proceso para matar.");
                    } else if (com == 4){ //comando prueba
                        com_valido = true;
                        nuevo=crearNodo(pid, gid, "file"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file2"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file3"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file4"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file5"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file6"); pid++; gid++; insertarFinal(listos,nuevo);
                    } else if (com == 5){ //comando fork
                        mvprintw(39, 0, "No hay procesos para copiar");
                    }
                    
                    else { //error al ingresar comando
                        move(37,2);
                        clrtoeol();
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
            while(i<=pc && fgets(linea, sizeof(linea), file) != NULL){
                i++;
                continue;
            }
           
            interrumpido=false; //Bandera para cada archivo
            strcpy(linea_original, "---");
            while (fgets(linea, sizeof(linea), file) != NULL) {
                linea[strcspn(linea, "\n\r")] = '\0';
                strcpy(linea_original, linea);//Para imprimir la linea original en PCB
                //usleep(1000000);
                imprimir_registros(pc, linea);
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                refresh();
                *ptr_pid = 0;
                
                ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++; 
                token = strtok(ptr, " \n");

                if (tokEND){ //Si hayamos un END...
                    if (token != NULL) { //Pero hay mas cosas despues
                        move(36,10);
                        clrtoeol();
                        mvprintw(36, 10, "Error: Contenido tras END en Renglon %d", pc);
                        proceso_a_terminar = desencolar(ejecutando); //Siguiendo la logica de Pedro

                        if (proceso_a_terminar != NULL) {
                            //strcpy(proceso_a_terminar->estado, "terminado*");
                            proceso_a_terminar->estadoTermino = 1;
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

                    if (strcmp(token, "END") == 0){
                        if (instEND()) {
                            tokEND = true;
                        } else {
                            tokEND = false;
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                //strcpy(proceso_a_terminar->estado, "terminado*");
                                proceso_a_terminar->estadoTermino = 1;
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            limpieza=true;
                            break;
                        }
                    } else if(strcmp(token, "JNZ") == 0){
                        int a = instJNZ(argumentos,proceso_actual,ptr_pc,ptr_pid);
                        if(a==1 || a == 2){
                            if(a==1){
                            rewind(file); //Regresa al inicio del archivo
                            int j=1;
                            while(j<=pc && fgets(linea,sizeof(linea), file) != NULL) {
                                j++;
                            }
                        }
                            quantum++;
                            proceso_actual->CPU = proceso_actual->CPU + 20;
                            proceso_actual->GCPU=proceso_actual->GCPU + 20;
                            aumentaGCPU(listos,proceso_actual->GID);

                            if (quantum == 3) {
                                guardaPCB(proceso_actual, pc, linea_original);
                                proceso_a_terminar = desencolar(ejecutando);
                                if (proceso_a_terminar!= NULL) {
                                    //strcpy(proceso_a_terminar->estado, "listos");
                                    proceso_a_terminar->estadoTermino = 0;
                                    insertarFinal(listos, proceso_a_terminar);
                                }
                                fclose(file);
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
                            //Mover los procesos fallidos a terminados
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                //strcpy(proceso_a_terminar->estado, "terminado*");
                                proceso_a_terminar->estadoTermino = 1;
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);

                            limpieza = true;
                            break; 
                        }
                    } else {
                        if (!ejecOperacion(token, argumentos, proceso_actual, ptr_pc, ptr_pid)) {
                            guardaPCB(proceso_actual,pc,linea_original);
                            mvprintw(34, 2, "ABORTADO: Error en renglon %d", pc);
                            mvprintw(36,2, "Motivo:");
                            //Mover los procesos fallidos a terminados
                            proceso_a_terminar = desencolar(ejecutando);
                            if (proceso_a_terminar != NULL) {
                                //strcpy(proceso_a_terminar->estado, "terminado*");
                                proceso_a_terminar->estadoTermino = 1;
                                insertarFinal(terminados, proceso_a_terminar);
                            }
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);

                            limpieza = true;
                            break; 
                        }
                    } 
                    //usleep(2000000);
                    if(*ptr_pid != -1){
                        pc++;
                    }
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
                            //strcpy(proceso_a_terminar->estado, "listos");
                            proceso_a_terminar->estadoTermino = 0;
                            insertarFinal(listos, proceso_a_terminar);
                        }
                        fclose(file);
                        proceso_actual = NULL;
                        fin_quantum = true;
                        limpieza = true;
                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                        refresh();
                        
                        break; 
                    }

                    //Movi el kbhit despues para que no haga falta aumentar pc dentro de kbhit
                    if(kbhit()){ //Cuando haya un teclazo
                        if(limpieza){ //tambien puede que no sea correco guardarlo asi
                            limpia_lineas();
                        }
                        interrumpido=true;
                        //Basto comentar el getch(), ahora ya no se ocupa dar enter y luego comando
                        //getch();

                        refresh();
                        move(40,2);
                        clrtoeol();
                        mvprintw(40, 2, ">");
                        echo();
                        comando[0] = '\0';
                        mvscanw(40,3,"%255[^\n]",comando);
                        noecho();
                        limpieza = true;
                        //strcpy(com_mata, comando);
                        com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst);

                        if (com == 1){
                            fclose(file);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                nuevo=crearNodo(pid, gid, archivo);
                                guardarTextoABinario(archivo, "disco_virtual.bin");
                                pid++;
                                gid++;
                                insertarFinal(listos,nuevo);
                                interrumpido = false;
                                continue; //Para seguir con el proceso actual y que no se cambie por el nuevo
                            } else {
                                move(37,2);
                                clrtoeol();
                                mvprintw(37,2,"Archivo no existente");
                                limpieza = true;
                                move(40,2);
                                clrtoeol();
                                refresh();
                            }      
                        
                        } else if(com == 3){
                            proceso_a_matar = extraerPID(ejecutando, pid_kill);
                            if(proceso_a_matar != NULL){
                                //strcpy(proceso_a_matar->estado, "terminados**");
                                proceso_a_matar->estadoTermino = 2;
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                break; 
                            } else{
                                proceso_a_matar = extraerPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    //strcpy(proceso_a_matar->estado, "terminados**");
                                    proceso_a_matar->estadoTermino = 2;
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } else {
                                    move(37,2);
                                    clrtoeol();
                                    mvprintw(37,2, "El PID asociado al proceso no existe.");
                                    mvprintw(27,2, "Ese proceso no existe o ya termino");
                                }
                            } 
                        } else if(com == 5){
                            proceso_a_copiar = buscaPID(ejecutando, pid_kill);
                            if(proceso_a_copiar != NULL){
                               nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo);
                               pid++;
                               nuevo->PC = num_inst;
                               nuevo->GCPU = proceso_a_copiar->GCPU;
                               insertarFinal(listos, nuevo);
                               imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            } else {
                                proceso_a_copiar = buscaPID(listos, pid_kill);
                                if(proceso_a_copiar != NULL) {
                                    nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo);
                                    pid++;
                                    nuevo->PC = num_inst;
                                    nuevo->GCPU = proceso_a_copiar->GCPU;
                                    insertarFinal(listos, nuevo);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } else {
                                    move(39,2);
                                    clrtoeol();
                                    mvprintw(39,2,"No existe el proceso asociado al PID o el proceso ya termino.");
                                }
                                
                            }

                        }
                        
                        else {
                            move(37,2);
                            clrtoeol();
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
                            fclose(file);
                            break;
                        }
                         
                    }

                } else {
                    move(36,2);
                    clrtoeol();
                    mvprintw(36, 2, "Token no valido: [%s]", token);
                    proceso_a_terminar = desencolar(ejecutando);
                    if (proceso_a_terminar != NULL) {
                        //strcpy(proceso_a_terminar->estado, "terminado*");
                        proceso_a_terminar->estadoTermino = 1;
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    limpieza = true;
                    break;
                }               
            }

            if(fin_quantum){ //esta bandera evita el doble cierre de archivos y el core dumpesd
                move(35, 2); clrtoeol();
                mvprintw(35, 2, "Quantum = 3. Cambio de proceso");
                //calculoPrioridades(listos,contarGrupos(listos,ejecutando,gid));
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                refresh();
            } else if(!interrumpido){ //Cuando el quantum no termina, osea no es multiplo de 3 el numero de instrucciones
                move(35, 2); clrtoeol();
                if (tokEND){
                    move(35,2);
                    clrtoeol();
                    mvprintw(35, 2, "Estado: Procesado con éxito.");
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);

                    if (proceso_a_terminar != NULL) {
                        //strcpy(proceso_a_terminar->estado, "terminado");
                        proceso_a_terminar->estadoTermino = 0;
                        insertarFinal(terminados, proceso_a_terminar);
                    }

                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                } else {
                    mvprintw(35, 2, "Estado: Error - Falto END o abortado.");
                    limpieza = true;
                    guardaPCB(proceso_actual,pc,linea_original);
                    proceso_a_terminar = desencolar(ejecutando);

                    if (proceso_a_terminar != NULL) {
                        //strcpy(proceso_a_terminar->estado, "terminado*");
                        proceso_a_terminar->estadoTermino = 1;
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    proceso_actual = NULL;
                }
                fclose(file);
                proceso_actual = NULL;
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                refresh();
            } else { //este era el else de cuando se ejecutaba el archivo hasta el final
                fclose(file);
                if(proceso_actual!=NULL){
                    guardaPCB(proceso_actual,pc,linea_original);
                }

                proceso_a_terminar = desencolar(ejecutando);
                if (proceso_a_terminar != NULL) {
                    //strcpy(proceso_a_terminar->estado, "terminado");
                    proceso_a_terminar->estadoTermino = 0;
                    insertarFinal(terminados, proceso_a_terminar);
                }
                //Si el proceso actual termino, ya no estamos ejecutando nada
                proceso_actual = NULL;
            }

        } else {
            mvprintw(34, 2, "El archivo NO existe.");
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