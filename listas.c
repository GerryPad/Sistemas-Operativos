#include <stdio.h>
#include <stdlib.h>

struct Nodo{
    int dato;
    struct Nodo *siguiente;
};

struct Nodo* crearCabecera(){
    struct Nodo *cabecera = malloc(sizeof(struct Nodo));
    cabecera->siguiente = NULL;
    return cabecera;
}

struct Nodo* crearNodo(int n){
    struct Nodo *nuevo = malloc(sizeof(struct Nodo));
    nuevo->dato = n;
    nuevo->siguiente = NULL;
    return nuevo;
}

void insertarInicio(struct Nodo *cabecera, struct Nodo *nuevo){
    nuevo->siguiente = cabecera->siguiente;
    cabecera->siguiente = nuevo;
}
void insertarFinal(struct Nodo *cabecera, struct Nodo *nuevo){
    struct Nodo *aux = cabecera;

    while(aux->siguiente != NULL){
        aux = aux->siguiente;
    }

    aux->siguiente = nuevo;
}

void imprimirLista(struct Nodo *cabecera){
    struct Nodo *aux = cabecera->siguiente;
    while(aux != NULL){
        printf("%d ", aux->dato);
        aux = aux->siguiente;
    }
    printf("\n");
}

struct Nodo * desencolar(struct Nodo *lista){
    struct Nodo *aux=lista->siguiente;
    if(aux==NULL){
        printf("La lista esta vacia\n");
        return NULL;
    }

    lista->siguiente=lista->siguiente->siguiente;
    aux->siguiente=NULL;
    return(aux);
}

int main(){

    struct Nodo *lista = crearCabecera();
    struct Nodo *ejecutando = crearCabecera();
    struct Nodo *terminados = crearCabecera();


    int n, opcion;

    while(1){
        
        printf("Inicio=1 Final=0 Desencolar=2: ");
        scanf("%d",&opcion);

        if(opcion == 1){
            printf("Ingrese numero (-1 para salir): ");
            scanf("%d",&n);
            if(n == -1){
            break;
            }
            struct Nodo *nuevo = crearNodo(n);
            insertarInicio(lista, nuevo);
        }else if(opcion==0){
            printf("Ingrese numero (-1 para salir): ");
            scanf("%d",&n);
             if(n == -1){
            break;
            }
            struct Nodo *nuevo = crearNodo(n);
            insertarFinal(lista, nuevo);
        }else if(opcion==2){
            desencolar(lista);
        }

        printf("Lista actual: ");
        imprimirLista(lista);
    }

    return 0;
}