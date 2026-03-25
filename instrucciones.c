#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include "instrucciones.h"

char *instruccion[] = {"MOV", "ADD", "SUB", "MUL", "DIV", "INC", "DEC", "END", NULL};

//Cada registro tiene un nombre y valor asociados.
Registro registros[] = {
    {"EAX", 0},
    {"EBX", 0},
    {"ECX", 0},
    {"EDX", 0},
    {NULL, 0}
};

//Funcion para comprobar si un token esta en el arreglo de instrucciones
bool validarToken(char *arr[], char *tok){
    if (tok == NULL) return false; //Necesario para ignorar lineas vacias
    for (int i = 0; arr[i] != NULL; i++){
        if (strcmp(arr[i], tok) == 0){
            return true;
        } 
    }
    return false;
}

//Funcion para comprobar si hemos leido un numero entero
bool esInt(char *s) {
    if (s == NULL || *s == '\0') return false;
    for (int i = 0; s[i] != '\0'; i++) {
        if(s[i] == '-' && i == 0) {
            continue;
        }
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

//Función para saber sobre que registro vamos a operar
Registro* buscaRegistro(char *nombre){
    for (int i = 0; registros[i].nombre != NULL; i++){
        if (strcmp(registros[i].nombre, nombre) == 0){
            return &registros[i];
        }
    }
    return NULL;
}

//Funciones para cada una de las instrucciones
bool instMOV(char *args){
    char op1[32], op2[32];
    int leidos = 0;
    Registro *reg1, *reg2;

    char *coma = strchr(args, ','); 

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { 
        move(24,10);
        clrtoeol();
        if (coma == NULL) {
            mvprintw(24, 10,"Falta la coma divisoria."); 
            return false;
        }
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10, "No se permiten espacios antes o despues de la coma");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            reg1->valor = reg2->valor;
        } else if (esInt(op2)) {
            reg1->valor = atoi(op2);
        } else {
            mvprintw(24, 10,"El segundo operando no es valido o hay demasiados argumentos.");
            return false;
        }
    } else {
        mvprintw(24, 10,"El operando uno de MOV no es un registro");
        return false;
    }
    return true;
}

bool instADD(char *args){
    char op1[32], op2[32];
    int leidos = 0;
    Registro *reg1, *reg2;

    char *coma = strchr(args, ','); 

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { 
        move(24,10);
        clrtoeol();
        if (coma == NULL) {
            mvprintw(24, 10,"Falta la coma divisoria."); 
            return false;
        }
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10, "No se permiten espacios antes o despues de la coma");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            reg1->valor = reg1->valor + reg2->valor;
        } else if (esInt(op2)) {
            reg1->valor = reg1->valor + atoi(op2); 
        } else {
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"El segundo operando no es valido o hay demasiados argumentos");
            return false;
        }
    } else {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"El operando uno de ADD no es un registro");
        return false;
    }
    return true;
}

bool instSUB(char *args){
    char op1[32], op2[32];
    int leidos = 0;
    Registro *reg1, *reg2;

    char *coma = strchr(args, ','); 

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { 
        move(24,10);
        clrtoeol();
        if (coma == NULL) {
            mvprintw(24, 10,"Falta la coma divisoria."); 
            return false;
        }
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10, "No se permiten espacios antes o despues de la coma");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            reg1->valor = reg1->valor - reg2->valor;
        } else if (esInt(op2)) {
            reg1->valor = reg1->valor - atoi(op2); 
        } else {
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"El segundo operando no es valido o hay demasiados argumentos.");
            return false;
        }
    } else {
        printf("El operando uno de SUB no es un registro");
        return false;
    }
    return true;
}

bool instMUL(char *args){
    char op1[32], op2[32];
    int leidos = 0;
    Registro *reg1, *reg2;

    char *coma = strchr(args, ','); 

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { 
        move(24,10);
        clrtoeol();
        if (coma == NULL) {
            mvprintw(24, 10,"Falta la coma divisoria."); 
            return false;
        }
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10, "No se permiten espacios antes o despues de la coma");
        return false;
    }

    reg1 = buscaRegistro(op1);

    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            reg1->valor = reg1->valor * reg2->valor;
        } else if (esInt(op2)) {
            reg1->valor = reg1->valor * atoi(op2);
        } else {
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"El segundo operando no es valido o hay demasiados argumentos.");
            return false;
        }
    } else {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"El operando uno de MUL no es un registro");
        return false;
    }
    return true;
}

bool instDIV(char *args){
    char op1[32], op2[32];
    int leidos = 0, divisor;
    Registro *reg1, *reg2;

    char *coma = strchr(args, ',');

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { 
        move(24,10);
        clrtoeol();
        if (coma == NULL) {
            mvprintw(24, 10,"Falta la coma divisoria."); 
            return false;
        }
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"No se permiten espacios antes o despues de la coma");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            if(reg2->valor == 0){
                move(24,10);
                clrtoeol();
                mvprintw(24, 10,"División por cero.");
                return false;
            }
            reg1->valor = reg1->valor / reg2->valor;
        } else if (esInt(op2)) {
            divisor = atoi(op2);
            if (divisor == 0){
                move(24,10);
                clrtoeol();
                mvprintw(24, 10,"División por cero.");
                return false;
            }
            reg1->valor = reg1->valor / divisor;
        } else {
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"El segundo operando no es valido o hay demasiados argumentos");
            return false;
        }
    } else {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"El operando uno de DIV no es un registro");
        return false;
    }
    return true;
}

bool instINC(char *args){
    char op1[32], basura[32];
    int leidos = 0;
    Registro *reg1;
    
    if (sscanf(args, "%31s %n", op1, &leidos) !=1) { 
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { 
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"Demasiados argumentos.");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg1->valor = reg1->valor + 1;
    } else {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"El operando de INC no es un registro");
        return false;
    }
    return true;
}

bool instDEC(char *args){
    char op1[32], basura[32];
    int leidos = 0;
    Registro *reg1;
    
    if (sscanf(args, "%31s %n", op1, &leidos) !=1) { 
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"Sintaxis incorrecta. Se esperaba 'REG,VALOR'");
        return false;
    }

    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { 
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"Demasiados argumentos.");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg1->valor = reg1->valor - 1;
    } else {
        move(24,10);
        clrtoeol();
        mvprintw(24, 10,"El operando uno de INC no es un registro.");
        return false;
    }
    return true;
}

bool instEND(){
    char *extra;    
    extra = strtok(NULL, " \n\t");
    if (extra != NULL){
        move(24,2);
        clrtoeol();
        mvprintw(24, 2,"Demasiados argumentos o instrucciones despues de END.");
        return false;
    }
    return true;
}

//Función "switch" para elegir la instruccion correspondiente. 
bool ejecOperacion(char *instruccion, char *args){
    if (strcmp(instruccion, "MOV") == 0){
        return instMOV(args);
    } else if (strcmp(instruccion, "ADD") == 0){
        return instADD(args);
    } else if (strcmp(instruccion, "SUB") == 0) {
        return instSUB(args);
    } else if (strcmp(instruccion, "MUL") == 0) {
        return instMUL(args);
    } else if (strcmp(instruccion, "DIV") == 0) {
        return instDIV(args);
    } else if (strcmp(instruccion, "INC") == 0) {
        return instINC(args);          
    } else if (strcmp(instruccion, "DEC") == 0) {
        return instDEC(args);
    } else {
        return false;
    }
}

int interpretar_comando(char *comando, char *archivo, int *ptr_pid) {
    char *arg, *basura, *cmd;

    cmd = strtok(comando, " \n");
    arg = strtok(NULL, " \n\r");

    if (cmd == NULL) return 0; //Para un enter sin comando

    if (strcmp(cmd, "salir") == 0) {
            if(arg != NULL){
                move(24,10);
                clrtoeol();
                mvprintw(24, 10,"Demasiados argumentos.");
                return 0;
            }
        return 1; 
    }

    if (strcmp(cmd, "ejecuta") == 0) {
        basura = strtok(NULL, " \n");
        if (arg == NULL){
            return -1;
        }

        if(basura != NULL){
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"Demasiados argumentos.");
            return 0;
        } else {
            //tenemos que intentar abrir el archivo para que no nos salga nada raro y no se creen archivos fantasmas
            FILE *f = fopen(arg, "r");
            if(f == NULL){ //esque intente poner file1 y si se creo
                move(24,10);
                clrtoeol();
                mvprintw(24,2,"Error: archivo no existe.");
                move(20,2);
                clrtoeol();
                return 0;
            }

            fclose(f);

            strcpy(archivo, arg); 
            return 2;
        }
    }

    if(strcmp(cmd, "mata") == 0){
        basura = strtok(NULL, " \n");
        if (arg == NULL){
            return -1;
        }

        if(basura != NULL){
            move(24,10);
            clrtoeol();
            mvprintw(24, 10,"Demasiados argumentos.");
            return 0;
        } else {
            *ptr_pid = atoi(arg); //OJO: la solución puede no servir para otros comandos
            //pid = atoi(arg);
            return 3;
        }
    }

    return 0;
}