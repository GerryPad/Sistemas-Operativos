#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PCB {
    int PID;
    int estado;
    int PC;
    char IR[50];
    int registros[4];
    struct PCB *siguiente;
} PCB;

typedef struct {
    PCB *cabecera;
} Lista;

Lista listos;
Lista ejecutando;
Lista terminados;

void insertarFinal(Lista *lista, PCB *nuevo);
PCB* crearPCB(int pid);
void imprimirLista(Lista *lista, const char *nombreLista);
PCB* desencolarFrente(Lista *lista);

void insertarFinal(Lista *lista, PCB *nuevo) {
    nuevo->siguiente = NULL;
    if (lista->cabecera == NULL) {
        lista->cabecera = nuevo;
    } else {
        PCB *aux = lista->cabecera;
        while (aux->siguiente != NULL) {
            aux = aux->siguiente;
        }
        aux->siguiente = nuevo;
    }
}

PCB* crearPCB(int pid) {
    PCB *nuevo = (PCB*)malloc(sizeof(PCB));
    nuevo->PID = pid;
    nuevo->estado = 0;
    nuevo->PC = 0;
    strcpy(nuevo->IR, "instruccion");
    for (int i = 0; i < 4; i++) {
        nuevo->registros[i] = 0;
    }
    nuevo->siguiente = NULL;
    return nuevo;
}

void imprimirLista(Lista *lista, const char *nombreLista) {
    printf("%s\n", nombreLista);
    PCB *aux = lista->cabecera;
    if (aux == NULL) {
        printf("(vacía)\n\n");
        return;
    }
    while (aux != NULL) {
        printf("PID=%d | estado=%d | PC=%d | IR=%s | registros=[%d,%d,%d,%d]\n\n",
            aux->PID,
            aux->estado,
            aux->PC,
            aux->IR,
            aux->registros[0],
            aux->registros[1],
            aux->registros[2],
            aux->registros[3]);
        aux = aux->siguiente;
    }
}

PCB* desencolarFrente(Lista *lista) {
    if (lista->cabecera == NULL) {
        return NULL;
    }
    PCB *extraido = lista->cabecera;
    lista->cabecera = lista->cabecera->siguiente;
    extraido->siguiente = NULL;
    return extraido;
}

int main() {
    listos.cabecera = NULL;
    ejecutando.cabecera = NULL;
    terminados.cabecera = NULL;

    char opcion[10];
    int pid;
    PCB *proceso;

    while (1) {
        printf("0: Añadir nuevo proceso a listos\n");
        printf("1: Mover de listos a ejecutando\n");
        printf("2: Mover de ejecutando a terminados\n");
        printf("3: Mover de ejecutando a listos\n");
        printf("4: Mover de listos a terminados\n");
        printf("Escribe 'salir' para terminar\n");
        printf("Opción: ");
        scanf("%s", opcion);

        if (strcmp(opcion, "salir") == 0) {
            break;
        }

        int op = atoi(opcion);

        switch (op) {
            case 0:
                printf("PID del nuevo proceso: ");
                scanf("%d", &pid);
                proceso = crearPCB(pid);
                insertarFinal(&listos, proceso);
                break;
            case 1:
                proceso = desencolarFrente(&listos);
                if (proceso == NULL) {
                    printf("Error: lista de listos vacia\n");
                } else {
                    proceso->estado = 1; //ejecutando
                    insertarFinal(&ejecutando, proceso);
                }
                break;
            case 2:
                proceso = desencolarFrente(&ejecutando);
                if (proceso == NULL) {
                    printf("Error: lista de ejecutando vacia\n");
                } else {
                    proceso->estado = 2; //terminado
                    insertarFinal(&terminados, proceso);
                }
                break;
            case 3:
                proceso = desencolarFrente(&ejecutando);
                if (proceso == NULL) {
                    printf("Error: lista de ejecutando vacia\n");
                } else {
                    proceso->estado = 0; //listo
                    insertarFinal(&listos, proceso);
                }
                break;
            case 4:
                proceso = desencolarFrente(&listos);
                if (proceso == NULL) {
                    printf("Error: lista de listos vacia\n");
                } else {
                    proceso->estado = 2; //terminado
                    insertarFinal(&terminados, proceso);
                }
                break;
            default:
                printf("Opcion no válida\n");
                break;
        }

        printf("\nListas:\n");
        imprimirLista(&listos, "Listos");
        imprimirLista(&ejecutando, "Ejecutando");
        imprimirLista(&terminados, "Terminados");
    }
    return 0;
}