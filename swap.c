#include <stdio.h>
//#include <unistd.h>
//#include <string.h>
//#include <stdbool.h>
//#include <stdlib.h>
//#include <curses.h>
#include <math.h>
//#include <time.h>
//#include <sys/select.h>
#include "nodo.h"

#define TAMANO_IR 64 
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) 
#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768

int guardarTextoABinario(const char *archivoTexto, FILE *bin, int pid, int gid, TablaMarcos *tms) {
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
                //if (strlen(linea) == 0) continue;
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
                tms[i].grupo = gid;
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

bool verificarEspacioEnSwap(const char *nombre_archivo, TablaMarcos *tms) {
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
        tmm[i].puntero = false;
        tmm[i].grupo = 0;
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
        tms[i].grupo = 0;
    }
    fseek(bin, 0, SEEK_SET);
}
