#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define TAMANO_IR 64 //Tamaño de instruccion
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) 
#define TOTAL_MARCOS_RAM 16
#define TOTAL_MARCOS_DISCO 32768

FILE *bin;

typedef struct {
    int num_marco;
    int propietario; // 0= libre, 0 != pid asignado
} TablaMarcos; 

TablaMarcos tmm[TOTAL_MARCOS_RAM];
TablaMarcos tms[TOTAL_MARCOS_DISCO];
char RAM[TOTAL_MARCOS_RAM*TAMANO_MARCO]; //1 = libre, 0 = ocupado


void guardarTextoABinario(const char *archivoTexto, FILE *bin) {
    FILE *txt = fopen(archivoTexto, "r");
    //FILE *bin = fopen(archivoBinario, "wb"); 

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
    //fclose(bin);
}

void cargarMarcoDesdeBinario(FILE *bin, int numeroMarco) {
    /*FILE *bin = fopen(archivoBinario, "rb");
    if (!bin) {
        printf("Error al abrir la memoria virtual.\n");
        return;
    }*/

    //Calcular la posición del marco
    long posicionBytes = (long)numeroMarco * TAMANO_MARCO;

    //Mover el puntero del archivo a esa posición
    if (fseek(bin, posicionBytes, SEEK_SET) != 0) {
        printf("Error: El marco %d no existe en la memoria virtual.\n", numeroMarco);
        //fclose(bin);
        return;
    }

    //Leer las 4 instrucciones (256 bytes totales)
    size_t leidos = fread(&RAM, sizeof(char), TAMANO_MARCO, bin);

    //Validar cuántas instrucciones reales pudimos leer (por si es el final del archivo)
    int instruccionesLeidas = leidos / TAMANO_IR;
    
    printf("\nContenido del Marco %d (%d bytes leídos) ---\n", numeroMarco, (int)leidos);
    for (int i = 0; i < instruccionesLeidas; i++) {
        printf("  Instruccion [%d] (Offset %d): %s\n", i + (numeroMarco * 4), i, RAM + (i*TAMANO_IR));
    }

    //fclose(bin);
}

void imprimeTMM(){
    for (int i=0; i<TOTAL_MARCOS_RAM; i++){
        printf("El marco %d tiene el propietario %d\n", tmm[i].num_marco, tmm[i].propietario);
    }
}

int main() {
    int total_instrucciones, total_marcos_necesarios, cnt_marcos_libres, pid=1, gid=1; 
    long tamano_bytes;
    //Conversion de archivo de texto a binario 
    for (int i=0; i<TOTAL_MARCOS_RAM; i++) {
        tmm[i].num_marco = i;
        tmm[i].propietario = 0;
    }
    bin = fopen("disco_virtual.bin", "wb+");
    guardarTextoABinario("file3", bin);
    //printf("Heloo\n");

    fseek(bin, 0, SEEK_END); //Movernos al final del archivo
    tamano_bytes = ftell(bin); //ftell devuelve en bytes la posicion actual del archivo, equivalente a la cantidad
    printf("Total de bytes: %ld\n", tamano_bytes);
    total_instrucciones = tamano_bytes/TAMANO_IR; //En teoria deberia ser forzosamente un entero
    printf("Numero de instrucciones: %d\n", total_instrucciones);
    total_marcos_necesarios = ((total_instrucciones + INSTRUCCIONES_POR_MARCO - 1)/INSTRUCCIONES_POR_MARCO); //Corregir para que pueda isar ceil
    printf("Numero de marcos necesarios: %d\n", total_marcos_necesarios);
    cnt_marcos_libres = 0;
    for(int m = 0; m < TOTAL_MARCOS_RAM; m++) {
        if(tmm[m].propietario == 0){
            cnt_marcos_libres++;
        } 
    }
    if (cnt_marcos_libres < total_marcos_necesarios) {  //FAltaria la logica de swapping
        printf("Se creo un proceso con el PID %d y el GID %d\n", pid, gid);//nuevo=crearNodo(pid,gid,archivo);
        pid++;
        gid++;
        printf("Se inserto al final de nuevos");//insertarFinal(nuevos, nuevo);
        //imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
        printf("Error: Memoria RAM insuficiente para el proceso."); //Quiatre esto cuando ya tenga la logica
    } else { //Aqui iria la logica de fallo de pagina
        int pagina_actual = 0;
        for(int i=0; i<TOTAL_MARCOS_RAM; i++){
            if(tmm[i].propietario == 0){
                fseek(bin, pagina_actual*TAMANO_MARCO, SEEK_SET);
                if(fread(RAM + (i*TAMANO_MARCO), sizeof(char), TAMANO_MARCO, bin) > 0) {
                    tmm[i].propietario = pid;
                    pagina_actual++;
                } else {
                    break;
                }
            }
        }
        //Modifciar el abrir/cerrar archivos solo una vez con el bin
        printf("Se creo un proceso nuevo con el PID %d y el GID %d\n", pid, gid);//nuevo=crearNodo(pid, gid, archivo);
        pid++;
        gid++;
        printf("Se inserto el proceso al final de suspendidos\n");//insertarFinal(suspendidos,nuevo); //Debe quedarse aqui un ratito aleatorio
        //imprimir_listas(ejecutando, listos, terminados, suspendidos, nuevos);
        printf("5 segundos de espera...\n");
        //usleep(5000000);
        printf("Se extrajo el proceso de suspendidos y se puso en listos\n");// = extraerPID(suspendidos, pid-1);
        //insertarFinal(listos, nuevo);
    }

    //Simulamos que el OS necesita cargar el Marco 1 y 2 a la RAM
    cargarMarcoDesdeBinario(bin, 0);
    cargarMarcoDesdeBinario(bin, 1); 
    cargarMarcoDesdeBinario(bin, 2);
    cargarMarcoDesdeBinario(bin, 3); 
    cargarMarcoDesdeBinario(bin, 4);

    imprimeTMM();

    fclose(bin);

    return 0;
}