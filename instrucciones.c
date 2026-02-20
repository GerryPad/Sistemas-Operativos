#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "instrucciones.h"

char *delimitadores = " ,\n";
char *delim2 = ",\n"; //Para no permitir la sintaxis con espacios entre registros
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
        if (strcmp(arr[i], tok) == 0){ //strcmp sirve para comparar cadenas
            return true;
        } 
    }
    return false;
}

//Funcion para comprobar si hemos leido un numero entero, incluidos los negativos
bool esInt(char *s) {
    if (s == NULL || *s == '\0') return false;
    for (int i = 0; s[i] != '\0'; i++) {
        if(s[i] == '-' && i == 0) { //Para reconocer el signo menos (-)
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
bool instMOV(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);
    Registro *reg1, *reg2;

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
    //Para revisar cuando hay mas de dos operandos, incluso aunque sean validos.
    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("MOV: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg2->valor;
            printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("MOV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = atoi(operando2); //atoi pasa de texto a int
            printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de MOV no es un registro\n");
        return false;
    }
    return true;
}

bool instADD(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);
    Registro *reg1, *reg2;

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }

    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("ADD: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor + reg2->valor;
            printf("ADD: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("ADD: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor + atoi(operando2); 
            printf("ADD: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de ADD no es un registro\n");
        return false;
    }
    return true;
}

bool instSUB(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);
    Registro *reg1, *reg2;

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
    
    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("SUB: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor - reg2->valor;
            printf("SUB: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("SUB: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor - atoi(operando2); 
            printf("SUB: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de SUB no es un registro\n");
        return false;
    }
    return true;
}

bool instMUL(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);
    Registro *reg1, *reg2;

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
    
    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);

    if (reg1 != NULL){
        reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            printf("MUL: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor * reg2->valor;
            printf("MUL: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            printf("MUL: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor * atoi(operando2);
            printf("MUL: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de MUL no es un registro\n");
        return false;
    }
    return true;
}

bool instDIV(){
    char *operando1, *operando2, *extra;
    operando1 = strtok(NULL, delim2);
    operando2 = strtok(NULL, delim2);
    Registro *reg1, *reg2;
    int divisor;

    if (operando1 == NULL || operando2 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
    
    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(operando2);
        if (reg2 != NULL){
            //Para comprobar que el divisor no sea cero
            if(reg2->valor == 0){
                printf("Error: División por cero.\n");
                return false;
            }
            printf("DIV: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor / reg2->valor;
            printf("DIV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(operando2)) {
            divisor = atoi(operando2);
            if (divisor == 0){
                printf("Error: División por cero.\n");
                return false;
            }
            printf("DIV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor / divisor;
            printf("DIV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else {
            printf("Error: El segundo operando no es valido.\n");
            return false;
        }
    } else {
        printf("Error: el operando uno de DIV no es un registro\n");
        return false;
    }
    return true;
}

bool instINC(){
    char *operando1, *extra;
    Registro *reg1;
    operando1 = strtok(NULL, delimitadores);

    if (operando1 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }

    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
        printf("INC: %s valor original %d\n", reg1->nombre, reg1->valor);
        reg1->valor = reg1->valor + 1;
        printf("INC: %s ahora vale %d\n", reg1->nombre, reg1->valor);
    } else {
        printf("Error: el operando de INC no es un registro\n");
        return false;
    }
    return true;
}

bool instDEC(){
    char *operando1, *extra;
    Registro *reg1;
    operando1 = strtok(NULL, delimitadores);
    
    if (operando1 == NULL) {
        printf("Error: Instrucción incompleta.\n");
        return false;
    }
    
    extra = strtok(NULL, delimitadores);
    if (extra != NULL){
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(operando1);
   
    if (reg1 != NULL){
        printf("DEC: %s valor original %d\n", reg1->nombre, reg1->valor);
        reg1->valor = reg1->valor - 1;
        printf("DEC: %s ahora vale %d\n", reg1->nombre, reg1->valor);
    } else {
        printf("Error: el operando uno de INC no es un registro\n");
        return false;
    }
    return true;
}

bool instEND(){
    char *extra;    
    extra = strtok(NULL, delimitadores);
    //Para comprobar que no haya mas tokens despues END en la linea final.
    if (extra != NULL){
        printf("Error: Demasiados argumentos o instrucciones despues de END.\n");
        return false;
    }
    return true;
}

//Función "switch" para elegir la instruccion correspondiente. 
bool ejecOperacion(char *instruccion){
    if (strcmp(instruccion, "MOV") == 0){
        return instMOV();
    } else if (strcmp(instruccion, "ADD") == 0){
        return instADD();
    } else if (strcmp(instruccion, "SUB") == 0) {
        return instSUB();
    } else if (strcmp(instruccion, "MUL") == 0) {
        return instMUL();
    } else if (strcmp(instruccion, "DIV") == 0) {
        return instDIV();
    } else if (strcmp(instruccion, "INC") == 0) {
        return instINC();          
    } else if (strcmp(instruccion, "DEC") == 0) {
        return instDEC();
    } else {
        return false;
    }
}