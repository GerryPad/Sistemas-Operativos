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
        if (s[i] < '0' || s[i] > '9') return false; //PORQUE RETORNAR FALSE
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
    char op1[32], op2[32], basura[32];
    int leidos = 0;
    Registro *reg1, *reg2;

    char *coma = strchr(args, ','); //Busca la posicion de la coma
    if (coma == NULL) {
        printf("Error: Falta la coma divisoria.\n"); //DESDE EL INICIO?
        return false;
    }

    // 2. Verificamos que el carácter inmediatamente anterior y posterior NO sea un espacio
    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        printf("Error: No se permiten espacios antes o despues de la coma.\n");
        return false;
    }

    // " %31[^,]" lee hasta hallar una coma
    // "," obliga a que exista la coma
    // " %31s" busca el segundo operando
    // 3. Si pasa la prueba, procedemos con el sscanf estricto
    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { //SERA QUE DA 1?
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            //printf("MOV: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg2->valor;
            //printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(op2)) {
            //printf("MOV: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = atoi(op2); //atoi pasa de texto a int
            //printf("MOV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
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

bool instADD(char *args){
    char op1[32], op2[32], basura[32];
    int leidos = 0;
    Registro *reg1, *reg2;
    char *coma = strchr(args, ','); //Busca la posicion de la coma
    if (coma == NULL) {
        printf("Error: Falta la coma divisoria.\n"); //DESDE EL INICIO?
        return false;
    }

    // 2. Verificamos que el carácter inmediatamente anterior y posterior NO sea un espacio
    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        printf("Error de sintaxis: No se permiten espacios antes o despues de la coma.\n");
        return false;
    }

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { //SERA QUE DA 1?
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            printf("ADD: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor + reg2->valor;
            printf("ADD: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(op2)) {
            printf("ADD: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor + atoi(op2); 
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

bool instSUB(char *args){
    char op1[32], op2[32], basura[32];
    int leidos = 0;
    Registro *reg1, *reg2;
    char *coma = strchr(args, ','); //Busca la posicion de la coma
    if (coma == NULL) {
        printf("Error: Falta la coma divisoria.\n"); //DESDE EL INICIO?
        return false;
    }

    // 2. Verificamos que el carácter inmediatamente anterior y posterior NO sea un espacio
    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        printf("Error de sintaxis: No se permiten espacios antes o despues de la coma.\n");
        return false;
    }

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { //SERA QUE DA 1?
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            printf("SUB: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor - reg2->valor;
            printf("SUB: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(op2)) {
            printf("SUB: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor - atoi(op2); 
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

bool instMUL(char *args){
    char op1[32], op2[32], basura[32];
    int leidos = 0;
    Registro *reg1, *reg2;
    char *coma = strchr(args, ','); //Busca la posicion de la coma
    if (coma == NULL) {
        printf("Error: Falta la coma divisoria.\n"); //DESDE EL INICIO?
        return false;
    }

    // 2. Verificamos que el carácter inmediatamente anterior y posterior NO sea un espacio
    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        printf("Error de sintaxis: No se permiten espacios antes o despues de la coma.\n");
        return false;
    }

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { //SERA QUE DA 1?
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(op1);

    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            printf("MUL: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor * reg2->valor;
            printf("MUL: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(op2)) {
            printf("MUL: %s valor original %d\n", reg1->nombre, reg1->valor);
            reg1->valor = reg1->valor * atoi(op2);
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

bool instDIV(char *args){
    char op1[32], op2[32], basura[32];
    int leidos = 0;
    Registro *reg1, *reg2;
    char *coma = strchr(args, ','); //Busca la posicion de la coma
    if (coma == NULL) {
        printf("Error: Falta la coma divisoria.\n"); //DESDE EL INICIO?
        return false;
    }

    // 2. Verificamos que el carácter inmediatamente anterior y posterior NO sea un espacio
    if (*(coma - 1) == ' ' || *(coma + 1) == ' ') {
        printf("Error de sintaxis: No se permiten espacios antes o despues de la coma.\n");
        return false;
    }

    if (sscanf(args, "%31[^,],%31s %n", op1, op2, &leidos) < 2) { //SERA QUE DA 1?
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }
    int divisor;

    reg1 = buscaRegistro(op1);
   
    if (reg1 != NULL){
        reg2 = buscaRegistro(op2);
        if (reg2 != NULL){
            //Para comprobar que el divisor no sea cero
            if(reg2->valor == 0){
                printf("Error: División por cero.\n");
                return false;
            }
            printf("DIV: %s valor original %d, %s valor original %d\n", reg1->nombre, reg1->valor, reg2->nombre, reg2->valor);
            reg1->valor = reg1->valor / reg2->valor;
            printf("DIV: %s ahora vale %d\n", reg1->nombre, reg1->valor);
        } else if (esInt(op2)) {
            divisor = atoi(op2);
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

bool instINC(char *args){
    char op1[32], basura[32];
    int leidos = 0;
    Registro *reg1;
    
    if (sscanf(args, "%31s %n", op1, &leidos) !=1) { //SERA QUE DA 1? //solo debe ser un operando por eso !=1
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
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

bool instDEC(char *args){
    char op1[32], basura[32];
    int leidos = 0;
    Registro *reg1;
    
    if (sscanf(args, "%31s %n", op1, &leidos) !=1) { //SERA QUE DA 1? //solo debe ser un operando por eso !=1
        printf("Error: Sintaxis incorrecta. Se esperaba 'REG,VALOR'\n");
        return false;
    }

    // Ahora es seguro usar 'leidos'
    if (leidos > 0 && sscanf(args + leidos, "%31s", basura) == 1) { //HAY MAS BASURA
        printf("Error: Demasiados argumentos.\n");
        return false;
    }

    reg1 = buscaRegistro(op1);
   
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
    extra = strtok(NULL, " \n\t");
    //Para comprobar que no haya mas tokens despues END en la linea final.
    if (extra != NULL){
        printf("Error: Demasiados argumentos o instrucciones despues de END.\n");
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