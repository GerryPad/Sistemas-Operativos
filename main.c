#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <math.h>
#include <time.h>
#include "instrucciones.h"
#include "ncurses.h"
#include "nodo.h"
#include "dispatch.h"
#include <sys/select.h>

#define TAMANO_IR 64 
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) 
#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768

FILE *bin;

TablaMarcos tmm[TOTAL_MARCOS_RAM];
TablaMarcos tms[TOTAL_MARCOS_DISCO];

char RAM[TOTAL_MARCOS_RAM*TAMANO_MARCO];

int guardarTextoABinario(const char *archivoTexto, FILE *bin, int pid) {
    FILE *txt = fopen(archivoTexto, "r");

    if (!txt || !bin) {
        mvprintw(34, 2, "Error al abrir los archivos.");
        return 0;
    }

    char linea[64];
    char bufferFijo[TAMANO_IR];

  // Leer el archivo de texto línea por línea
    int num_instrucciones = 0;
    int num_pagina = 0;

    for (int i=0; i<TOTAL_MARCOS_DISCO; i++){
        if(tms[i].propietario == 0){
            long posicion = (long)i * INSTRUCCIONES_POR_MARCO * TAMANO_IR;
            fseek(bin, posicion, SEEK_SET);
            int contador_lineas = 0;
            while(contador_lineas<INSTRUCCIONES_POR_MARCO && fgets(linea, sizeof(linea), txt)){
                linea[strcspn(linea, "\r\n")] = 0;
                //Saltar líneas vacías
                if (strlen(linea) == 0) continue;
                //Llenamos el marco inicialmente con 0's
                memset(bufferFijo, 0, TAMANO_IR);
                //Copiamos el texto de la instrucción al buffer seguro
                strncpy(bufferFijo, linea, TAMANO_IR - 1);
                //Escribimos exactamente 64 bytes en el archivo binario
                fwrite(bufferFijo, sizeof(char), TAMANO_IR, bin);
                num_instrucciones++;
                contador_lineas++;
            }

            if (contador_lineas>0){
                tms[i].propietario = pid;
                tms[i].num_pagina = num_pagina;
                num_pagina++;
                //tms[i].valida=1 averiguar para que es esto
                if (feof(txt)){
                    break;
                }
            }
            //tms[i].propietario = pid;
        } else {
            continue;
        }
    }

    fclose(txt);
    return num_instrucciones; 
}

int cuentaMarcosNecesarios(const char *nombre_archivo){
    FILE *txt = fopen(nombre_archivo, "r");
    if (txt == NULL) {
        mvprintw(35, 2, "Error: No se pudo abrir el archivo.");
        return -1;
    }

    int lineas = 0;
    char buffer[64]; 
    while (fgets(buffer, sizeof(buffer), txt) != NULL) {
        lineas++;
    }
    fclose(txt);

    //Calcular cuantas paginas va a necesitar
    int marcos_necesarios = ceil((float) lineas  / INSTRUCCIONES_POR_MARCO);
    return marcos_necesarios;
}

bool verificarEspacioEnSwap(const char *nombre_archivo) {
   FILE *txt = fopen(nombre_archivo, "r");
    if (txt == NULL) {
        mvprintw(35, 2, "Error: No se pudo abrir el archivo.");
        return false;
    }

    int lineas = 0;
    char buffer[64]; 
    while (fgets(buffer, sizeof(buffer), txt) != NULL) {
        lineas++;
    }
    fclose(txt);

    //Calcular cuantas paginas va a necesitar
    int marcos_necesarios = ceil((float) lineas  / INSTRUCCIONES_POR_MARCO);

    //Contar cuántos marcos libres (propietario == 0) quedan en el Disco (Swap)
    int marcos_libres_disco = 0;
    for (int i = 0; i < TOTAL_MARCOS_DISCO; i++) {
        if (tms[i].propietario == 0) {
            marcos_libres_disco++;
            if(marcos_libres_disco == marcos_necesarios) break;
        }
    }

    //Evaluar si cabe
    if (marcos_necesarios > marcos_libres_disco) {
        mvprintw(37, 2,"[ERROR] El proceso requiere mas paginas de los disponibles en swap.");
        return false;
    }

    //Si cabe, retornamos true   
    return true;
}

int cargarARAM(int pid, int num_pagina, FILE *bin) {
    int marco_disco = -1;
    int contador_paginas = 0;

    //Buscar en la TMS la ubicacion fisica de la pagina
    for (int i = 0; i < TOTAL_MARCOS_DISCO; i++) {
        if (tms[i].propietario == pid) {
            if (contador_paginas == num_pagina) {
                marco_disco = i;
                break;
            }
            contador_paginas++;
        }
    }

    if (marco_disco == -1) {
        mvprintw(36, 2, "Error: La pagina %d del PID %d no existe en SWAP.", num_pagina, pid);
        return -1;
    }

    //Buscar un marco libre en la TMM 
    for (int marco_ram = 0; marco_ram < TOTAL_MARCOS_RAM; marco_ram++) {
        if (tmm[marco_ram].propietario == 0) {
            
            //Copiar datos del disco a la RAM
            fseek(bin, marco_disco * TAMANO_MARCO, SEEK_SET);
            fread(RAM + (marco_ram * TAMANO_MARCO), 1, TAMANO_MARCO, bin);
            
            //Actualizar TMM 
            tmm[marco_ram].propietario = pid;
            tmm[marco_ram].num_pagina = num_pagina;
            mvprintw(39, 2, "Pagina %d del PID %d cargada en Marco RAM %d", num_pagina, pid, marco_ram);
            return marco_ram; 
        }
    }

    mvprintw(38, 2, "Fallo de pagina: No hay marcos libres en RAM");
    return -1;
}

struct Nodo* buscarHerederoGID(struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos, int gid, int pid_actual) {
    struct Nodo *aux_l = listos->siguiente;
    struct Nodo *aux_e = ejecutando->siguiente;
    struct Nodo *aux_s = suspendidos->siguiente;

    while(aux_l != NULL) {
        if(aux_l->GID == gid && aux_l->PID != pid_actual){
            return aux_l;
        } 
        aux_l = aux_l->siguiente;
    }

    while(aux_e != NULL) {
        if(aux_e->GID == gid && aux_e->PID != pid_actual){
            return aux_e;
        } 
        aux_e = aux_e->siguiente;
    }

    while(aux_s != NULL) {
        if(aux_s->GID == gid && aux_s->PID != pid_actual){
            return aux_s;
        } 
        aux_s = aux_s->siguiente;
    }
    return NULL; //No hay nadie mas en el grupo
}

//Funcion que borraria paginas de la TMS y TMM, aun no las borra de RAM ni de disco
//Aun no consideramos que pasa si otro proceso creado con fork las necesita
void eliminarPaginas(struct Nodo *proceso, TablaMarcos *tms, TablaMarcos *tmm, struct Nodo *listos, struct Nodo *ejecutando, struct Nodo *suspendidos){
    int pid_busqueda = proceso->PID;
    struct Nodo *heredero = buscarHerederoGID(listos, ejecutando, suspendidos, proceso->GID, pid_busqueda);
    char buffer[TAMANO_MARCO];
    memset(buffer,0,sizeof(buffer));
    //int procesos_mismo_gid = cuentaPorGID(listos, ejecutando, suspendidos, proceso->GID, proceso->PID);
    
    //No la esta eliminando cuadno ya es el ultimo proceso y acaba
    if(heredero != NULL) { //Hay mas de un proceso con el mismo GID
        //tal vez crear funcion que cuente cuantos procesos tienen el mismo GID?
 
        for (int i=0; i<TOTAL_MARCOS_RAM; i++){
            if(tmm[i].propietario == pid_busqueda){
                tmm[i].propietario = heredero->PID;
                //tmm[i].num_pagina = -1; 
            }
        }

        for (int i=0; i<TOTAL_MARCOS_DISCO; i++){
            if(tms[i].propietario == pid_busqueda){
                tms[i].propietario = heredero->PID;
                //tms[i].num_pagina = -1;
            }
        }
    } else { //No hay mas procesos con el mismo GID
        for (int i=0; i<TOTAL_MARCOS_RAM; i++){
            if(tmm[i].propietario == pid_busqueda){
                tmm[i].propietario = 0;
                tmm[i].num_pagina = -1; 
                tmm[i].usado_recien = 0;
                //fread(RAM + (marco_ram * TAMANO_MARCO), 1, TAMANO_MARCO, bin);
                
                memset((RAM + TAMANO_MARCO * i) ,0,TAMANO_MARCO);
                
            }

        }

        for(int i=0; i<TOTAL_MARCOS_DISCO; i++){
            if(tms[i].propietario == pid_busqueda){ // 0 1 2 3
                tms[i].propietario = 0;
                tms[i].num_pagina = -1;
                fseek(bin,TAMANO_MARCO*i,SEEK_SET); //en el binario a partir del 0 256*3 = 768
                fwrite(buffer, sizeof(char), TAMANO_MARCO, bin);
            }
        }
        fflush(bin); //vacía el búfer hacia el archivo físico
    }

    for (int i=0; i<proceso->num_paginas; i++){
        proceso->tmp[i].num_marco_disco = -1;
        proceso->tmp[i].num_marco_ram = -1;
    }
    //Si hago 2 forks a un proceso y quiero matar al segundo fork no se deberian asignar sus valores a nadie y su TMP se iria a -1
}

void porcentajeDiscoRAM(){
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

    float porcentajeRAM = 0;
    float porcentajeDISCO = 0;

    porcentajeRAM = (ocupado_ram * 100.0) * (0.0625);
    porcentajeDISCO = (ocupado_disco * 100) * (0.0000305176);
    mvprintw(28, 162, "Uso RAM: %.2f%%", porcentajeRAM);
    mvprintw(29, 162, "Uso DISCO: %.2f%%", porcentajeDISCO);

}

void iniciarDiscoYTablas(TablaMarcos *tms, TablaMarcos *tmm, FILE *bin){
    // Mover el cursor del archivo a la posición deseada menos 1 byte
    fseek(bin, 8388608 - 1, SEEK_SET);

    // Escribir un byte nulo para definir el tamaño en el disco
    fputc('\0', bin);

    for (int i=0; i<TOTAL_MARCOS_RAM; i++) {
        tmm[i].num_marco = i;
        tmm[i].propietario = 0;
        tmm[i].num_pagina = -1;
        tmm[i].usado_recien = 0;
        if(i==TOTAL_MARCOS_RAM-1){
            tmm[i].siguiente = &tmm[0];
        } else {
           tmm[i].siguiente = &tmm[i+1]; 
        }

    }

    for (int i=0; i<TOTAL_MARCOS_DISCO; i++) {
        tms[i].num_marco = i;
        tms[i].propietario = 0;
        tms[i].num_pagina = -1;
    }
    fseek(bin, 0, SEEK_SET);
}

void sacarSuspendidos(struct Nodo *suspendidos, struct Nodo *listos){
    struct Nodo *aux_s = suspendidos->siguiente;
    struct Nodo *proceso_a_mover = NULL;

    while(aux_s != NULL){
        if(difftime(time(NULL), aux_s->hora_entrada) >= aux_s->tiempo_espera) {
            proceso_a_mover = extraerPID(suspendidos, aux_s->PID);
            insertarFinal(listos, proceso_a_mover);
        }
        aux_s = aux_s->siguiente;
    }
}

struct TablaMarcos *algoritmoReloj(TablaMarcos *manecilla){
    while(manecilla->usado_recien == 1){
        manecilla->usado_recien = 0;
        manecilla = manecilla->siguiente;
    }

    tmm[manecilla->num_marco].propietario = 0;
    tmm[manecilla->num_marco].num_pagina = -1;
    memset((RAM + TAMANO_MARCO * manecilla->num_marco) ,0,TAMANO_MARCO);

    return manecilla; //Este es el marco con la pagina a desalojar
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
    struct Nodo *proceso_a_suspender  = NULL;

    struct TablaMarcos *manecilla_reloj = &tmm[0];

    char archivo[64], linea[TAMANO_IR + 1], comando[256], linea_original[128];//, com_mata[256]; //Buffers para leer nombre y linea del archivo.
    int pc, com, pid=1, gid=1, pid_kill=0, num_inst = 0, quantum = 0;
    int *ptr_pid = &pid_kill, *ptr_inst = &num_inst, *ptr_pc=&pc; //com es para hacer un "switch" 
    int total_instrucciones, total_marcos_necesarios;// cnt_marcos_libres;
    char *token, *ptr, *argumentos;
    bool tokEND, com_valido, interrumpido; //com_valido es para comprobar la existencia del comando
    bool fin_quantum, limpieza = false; 
    bool page_fault = false;

    srand(time(NULL));

    bin = fopen("disco_virtual.bin", "wb+");
    if (!bin) {
        perror("fopen");
    }

    iniciarDiscoYTablas(tms, tmm, bin);
   
    
    initscr();
    do{
        tokEND = false;

        sacarSuspendidos(suspendidos, listos);
        if(ejecutando->siguiente == NULL){ //Cambiar el uso de la bandera pedir archivo
            if(listos->siguiente != NULL || suspendidos->siguiente != NULL){
                calculoPrioridades(listos,contarGrupos(listos,ejecutando,gid));
                actualizaCGPU(suspendidos->siguiente);
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                //usleep(3000000);
                
                proceso_actual = planificador(listos, ejecutando); //Hacer que el planificador te de el primero de listos

                if (proceso_actual != NULL) {
                    pc = restauraPCB(proceso_actual, archivo); 
                } else {
                    sacarSuspendidos(suspendidos, listos);
                    if(kbhit()){ //Cuando haya un teclazo
                        if(limpieza){ //tambien puede que no sea correco guardarlo asi
                            limpia_lineas();
                        }

                        interrumpido=true;
                        refresh();
                        mvprintw(40, 2, "%-28s", ""); 
                        mvprintw(40, 2, ">");
                        echo();
                        comando[0] = '\0';
                        mvscanw(40,3,"%255[^\n]",comando);
                        noecho();
                        limpieza = true;
                        com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst);

                        if (com == 1){
                            fclose(bin);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                if(verificarEspacioEnSwap(archivo)){
                                    total_instrucciones = guardarTextoABinario(archivo, bin, pid);
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
                                //strcpy(proceso_a_matar->estado, "terminados**");
                                proceso_a_matar->estadoTermino = 2;
                                eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_matar);
                                porcentajeDiscoRAM();
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                break; 
                            } else{
                                proceso_a_matar = extraerPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    //strcpy(proceso_a_matar->estado, "terminados**");
                                    proceso_a_matar->estadoTermino = 2;
                                    eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos);
                                    imprimirTmm(tmm);
                                    imprimirTms(tms);
                                    imprimirTmp(proceso_a_matar);
                                    porcentajeDiscoRAM();
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } else {
                                    proceso_a_matar = extraerPID(suspendidos, pid_kill);
                                    if(proceso_a_matar != NULL){
                                        //strcpy(proceso_a_matar->estado, "terminados**");
                                        proceso_a_matar->estadoTermino = 2;
                                        eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos);
                                        imprimirTmm(tmm);
                                        imprimirTms(tms);
                                        imprimirTmp(proceso_a_matar);
                                        porcentajeDiscoRAM();
                                        insertarFinal(terminados,proceso_a_matar);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    } else {
                                        mvprintw(37,2, "El PID asociado al proceso no existe.");
                                        mvprintw(27,2, "Ese proceso no existe o ya termino");
                                    }
                                    
                                }
                            } 
                        } else if(com == 5){
                            proceso_a_copiar = buscaPID(ejecutando, pid_kill);
                            if(proceso_a_copiar != NULL){
                                if(verificarEspacioEnSwap(archivo)){
                                    total_marcos_necesarios = cuentaMarcosNecesarios(proceso_a_copiar->archivo);
                                    nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, total_marcos_necesarios);
                                    actualizaTMP(nuevo, tms);
                                    imprimirTms(tms);
                                    pid++;
                                    nuevo->PC = num_inst;
                                    nuevo->GCPU = proceso_a_copiar->GCPU;
                                    insertarFinal(listos, nuevo);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } //creo que aqui falta mandarlo a nuevos 
                              
                            } else { 
                                proceso_a_copiar = buscaPID(listos, pid_kill);
                                if(proceso_a_copiar != NULL) {
                                    if(verificarEspacioEnSwap(archivo)){
                                        total_marcos_necesarios = cuentaMarcosNecesarios(proceso_a_copiar->archivo);
                                        nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, total_marcos_necesarios);
                                        actualizaTMP(nuevo, tms);
                                        imprimirTms(tms);
                                        pid++;
                                        nuevo->PC = num_inst;
                                        nuevo->GCPU = proceso_a_copiar->GCPU;
                                        insertarFinal(listos, nuevo);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    }
                                } else {
                                    mvprintw(39,2,"No existe el proceso asociado al PID o el proceso ya termino.");
                                }
                                
                            }

                        }
                        
                        else {
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
            } else {
                com_valido = false; //Suponemos de entrada que el comando no es valido

                while (!com_valido){ //Solicitamos comando hasta que haya uno valido
                    mvprintw(40, 2, "%-28s", ""); 
                    mvprintw(40,2, ">");
                    echo();
                    comando[0] = '\0';
                    getstr(comando);
                    noecho();
                    mvprintw(40, 2, "%-28s", ""); 
                    refresh();
                
                    com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst); 
                    limpieza = true;

                    if (com == 1){ //comando salir
                        endwin();
                        fclose(bin);
                        return 0;
                    } else if (com == 2){ //comando ejecuta
                        com_valido = true;
                        if(verificarEspacioEnSwap(archivo)){
                            total_instrucciones = guardarTextoABinario(archivo, bin, pid);
                            total_marcos_necesarios = ceil((float)total_instrucciones/INSTRUCCIONES_POR_MARCO); 
                            nuevo=crearNodo(pid,gid,archivo, total_marcos_necesarios);
                            actualizaTMP(nuevo, tms);
                            pid++;
                            gid++;
                            insertarFinal(listos, nuevo);
                            imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                            refresh();
                            //continue;

                            /*//Aqui iria la logica de fallo de pagina
                               int total_paginas = total_marcos_necesarios;

                                for (int p = 0; p < total_paginas; p++) {
                                    int resultado = cargarARAM(pid, p, bin);
                                    if (resultado == -1) {
                                        //Logica para algoritmo de reemplazo
                                        break; 
                                    }
                                }
                                //Modifciar el abrir/cerrar archivos solo una vez con el bin
                                nuevo=crearNodo(pid, gid, archivo);
                                pid++;
                                gid++;
                                insertarFinal(suspendidos,nuevo); //Debe quedarse aqui un ratito aleatorio
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                usleep(5000000);
                                nuevo = extraerPID(suspendidos, pid-1);
                                insertarFinal(listos, nuevo);
                            */
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

                    } else if(com == 3){ //comando mata
                        mvprintw(37, 2, "No hay ningun proceso para matar.");
                    } else if (com == 4){ //comando prueba
                        com_valido = true;
                        /*nuevo=crearNodo(pid, gid, "file"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file2"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file3"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file4"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file5"); pid++; gid++; insertarFinal(listos,nuevo);
                        nuevo=crearNodo(pid, gid, "file6"); pid++; gid++; insertarFinal(listos,nuevo);*/
                    } else if (com == 5){ //comando fork
                        mvprintw(39, 0, "No hay procesos para copiar");
                    }
                    
                    else { //error al ingresar comando
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

        if (proceso_actual != NULL) { //Cambiar condicional por proceso_actual != NULL?
            quantum = 0;
            fin_quantum = false; //para saber porque motivo cerramos proceso
            contarGrupos(listos, ejecutando, gid);
           
            interrumpido=false; //Bandera para cada archivo
            strcpy(linea_original, "---");
            sacarSuspendidos(suspendidos, listos);
            while (quantum < 3) {
                memset(linea, 0, TAMANO_IR);
                int pag_actual = pc / INSTRUCCIONES_POR_MARCO;
                if (pag_actual >= proceso_actual->num_paginas) {
                    mvprintw(36, 10, "Error: Se alcanzo el fin de memoria sin encontrar END.");
                    tokEND = false;  // Marcamos que fue un error
                    limpieza = true;
                    break;           // Rompemos el ciclo inmediatamente
                }
                
                int desplazamiento = pc % INSTRUCCIONES_POR_MARCO;
                int marco_ram = proceso_actual->tmp[pag_actual].num_marco_ram;
                int pos_fisica=0;
                
                if (marco_ram == -1) {
                    manecilla_reloj = algoritmoReloj(manecilla_reloj);
                    page_fault = true;
                    proceso_a_suspender = desencolar(ejecutando);
                    if (proceso_a_suspender != NULL) {
                        insertarFinal(suspendidos, proceso_a_suspender);
                        proceso_a_suspender->hora_entrada = time(NULL);
                        proceso_a_suspender->tiempo_espera = rand() % (9) + 2; //%(9)+2

                        int marco_ram_nuevo = cargarARAM(proceso_a_suspender->PID, pag_actual, bin);
                        if (marco_ram_nuevo != -1) {
                            proceso_a_suspender->tmp[pag_actual].num_marco_ram = marco_ram_nuevo;
                            tmm[marco_ram_nuevo].usado_recien = 1;
                        } else {
                            //manecilla_reloj = algoritmoReloj(manecilla_reloj);
                            //mvprintw(36, 2, "No hay marcos libres en RAM. Implementar reemplazo.");

                        }
                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                        imprimirTmm(tmm);
                        porcentajeDiscoRAM();
                        refresh();
                    }
                    proceso_actual = NULL;
                    limpieza = true;
                    break;  // Sale del while de quantum
                }

                if(marco_ram != -1) {
                    pos_fisica = (marco_ram * TAMANO_MARCO) + (desplazamiento * TAMANO_IR);
                    tmm[marco_ram].usado_recien = 1;
                    memcpy(linea, &RAM[pos_fisica], TAMANO_IR);
                    linea[TAMANO_IR] = '\0';
                }
                //
                strcpy(linea_original, linea);//Para imprimir la linea original en PCB
                //usleep(1000000);


                
                imprimir_registros(pc, linea);
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                imprimirTmm(tmm);
                imprimirTmp(proceso_actual);
                imprimirTms(tms);

                //break;
                refresh();
                *ptr_pid = 0;
                
                ptr = linea;
                while (*ptr == ' ' || *ptr == '\t') ptr++; 
                token = strtok(ptr, " \r\n\t");

/*                    mvprintw(0, 0, "DEBUG -> PC: %d | Linea cruda: [%s] | Token extraido: [%s]    ", 
                            pc, linea_original, token != NULL ? token : "NULO");
                    refresh();
                    usleep(500000);
*/
                if (tokEND){ //Si hayamos un END...
                    if (token != NULL) { //Pero hay mas cosas despues
                        mvprintw(36, 10, "Error: Contenido tras END en Renglon %d", pc);
                        proceso_a_terminar = desencolar(ejecutando); //Siguiendo la logica de Pedro

                        if (proceso_a_terminar != NULL) {
                            //strcpy(proceso_a_terminar->estado, "terminado*");
                            proceso_a_terminar->estadoTermino = 1;
                            eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                            imprimirTmm(tmm);
                            imprimirTms(tms);
                            imprimirTmp(proceso_a_terminar);
                            porcentajeDiscoRAM();
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
                            //CASO 1: Hay basura después del END
                            mvprintw(36, 10, "Error: Contenido tras END en Renglon %d", pc);
                            tokEND = false; 
                            limpieza = true;
                            break; // Salimos y el código de afuera lo manda a estadoTermino = 1
                        } else {
                            //CASO 2: Es un END limpio
                            if (instEND()) {
                                tokEND = true; 
                                limpieza = true;
                                break; // Salimos y el código de afuera lo manda a estadoTermino = 0
                            } else {
                                tokEND = false;
                                limpieza = true;
                                break;
                            }
                        }
                    } else if(strcmp(token, "JNZ") == 0){
                        int aux = instJNZ(argumentos,proceso_actual, pc);
                        if(aux != -1){
                            pc = aux;
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
                                eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_terminar);
                                porcentajeDiscoRAM();
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
                                eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_terminar);
                                porcentajeDiscoRAM();
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
                        mvprintw(40, 2, "%-28s", ""); 
                        mvprintw(40, 2, ">");
                        echo();
                        comando[0] = '\0';
                        mvscanw(40,3,"%255[^\n]",comando);
                        noecho();
                        limpieza = true;
                        com = interpretar_comando(comando, archivo, ptr_pid, ptr_inst);

                        if (com == 1){
                            fclose(bin);
                            endwin();
                            return 0;
                        } else if (com == 2){
                            if (access(archivo, F_OK) == 0){
                                if(verificarEspacioEnSwap(archivo)){
                                    total_instrucciones = guardarTextoABinario(archivo, bin, pid);
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
                                //strcpy(proceso_a_matar->estado, "terminados**");
                                proceso_a_matar->estadoTermino = 2;
                                eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos);
                                imprimirTmm(tmm);
                                imprimirTms(tms);
                                imprimirTmp(proceso_a_matar);
                                porcentajeDiscoRAM();
                                insertarFinal(terminados,proceso_a_matar);
                                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                break; 
                            } else{
                                proceso_a_matar = extraerPID(listos, pid_kill);
                                if(proceso_a_matar != NULL){
                                    //strcpy(proceso_a_matar->estado, "terminados**");
                                    proceso_a_matar->estadoTermino = 2;
                                    eliminarPaginas(proceso_a_matar, tms, tmm, listos, ejecutando, suspendidos);
                                    imprimirTmm(tmm);
                                    imprimirTms(tms);
                                    imprimirTmp(proceso_a_matar);
                                    porcentajeDiscoRAM();
                                    insertarFinal(terminados,proceso_a_matar);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } else {
                                    mvprintw(37,2, "El PID asociado al proceso no existe.");
                                    mvprintw(27,2, "Ese proceso no existe o ya termino");
                                }
                            } 
                        } else if(com == 5){
                            proceso_a_copiar = buscaPID(ejecutando, pid_kill);
                            if(proceso_a_copiar != NULL){
                                if(verificarEspacioEnSwap(archivo)){
                                    total_marcos_necesarios = cuentaMarcosNecesarios(proceso_a_copiar->archivo);
                                    nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, total_marcos_necesarios);
                                    actualizaTMP(nuevo, tms);
                                    imprimirTms(tms);
                                    pid++;
                                    nuevo->PC = num_inst;
                                    nuevo->GCPU = proceso_a_copiar->GCPU;
                                    insertarFinal(listos, nuevo);
                                    imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                } //creo que aqui falta mandarlo a nuevos 
                              
                            } else { 
                                proceso_a_copiar = buscaPID(listos, pid_kill);
                                if(proceso_a_copiar != NULL) {
                                    if(verificarEspacioEnSwap(archivo)){
                                        total_marcos_necesarios = cuentaMarcosNecesarios(proceso_a_copiar->archivo);
                                        nuevo=crearNodo(pid, proceso_a_copiar->GID, proceso_a_copiar->archivo, total_marcos_necesarios);
                                        actualizaTMP(nuevo, tms);
                                        imprimirTms(tms);
                                        pid++;
                                        nuevo->PC = num_inst;
                                        nuevo->GCPU = proceso_a_copiar->GCPU;
                                        insertarFinal(listos, nuevo);
                                        imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                                    }
                                } else {
                                    mvprintw(39,2,"No existe el proceso asociado al PID o el proceso ya termino.");
                                }
                                
                            }

                        }
                        
                        else {
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
                        //strcpy(proceso_a_terminar->estado, "terminado*");
                        proceso_a_terminar->estadoTermino = 1;
                        eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                        imprimirTmm(tmm);
                        imprimirTms(tms);
                        imprimirTmp(proceso_a_terminar);
                        porcentajeDiscoRAM();
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                    limpieza = true;
                    break;
                }               
            }

            sacarSuspendidos(suspendidos, listos);
            if(page_fault){
                page_fault = false;
            } else if(fin_quantum){ //esta bandera evita el doble cierre de archivos y el core dumpesd
                mvprintw(35, 2, "Quantum = 3. Cambio de proceso");
                //calculoPrioridades(listos,contarGrupos(listos,ejecutando,gid));
                imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
                refresh();
            } else if(!interrumpido){ //Cuando el quantum no termina, osea no es multiplo de 3 el numero de instrucciones
                if (tokEND){
                    mvprintw(35, 2, "Estado: Procesado con éxito.");
                    if(proceso_actual!=NULL) {
                        guardaPCB(proceso_actual,pc,linea_original);
                        proceso_a_terminar = desencolar(ejecutando);

                        if (proceso_a_terminar != NULL) {
                            //strcpy(proceso_a_terminar->estado, "terminado");
                            proceso_a_terminar->estadoTermino = 0;
                            eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                            imprimirTmm(tmm);
                            imprimirTms(tms);
                            imprimirTmp(proceso_a_terminar);
                            porcentajeDiscoRAM();
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
                            //strcpy(proceso_a_terminar->estado, "terminado*");
                            proceso_a_terminar->estadoTermino = 1;
                            eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                            imprimirTmm(tmm);
                            imprimirTms(tms);
                            imprimirTmp(proceso_a_terminar);
                            porcentajeDiscoRAM();
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
                        //strcpy(proceso_a_terminar->estado, "terminado");
                        proceso_a_terminar->estadoTermino = 0;
                        eliminarPaginas(proceso_a_terminar, tms, tmm, listos, ejecutando, suspendidos);
                        imprimirTmm(tmm);
                        imprimirTms(tms);
                        imprimirTmp(proceso_a_terminar);
                        porcentajeDiscoRAM();
                        insertarFinal(terminados, proceso_a_terminar);
                    }
                }
                //Si el proceso actual termino, ya no estamos ejecutando nada
                proceso_actual = NULL;
            }

        } else {
            sacarSuspendidos(suspendidos, listos);
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