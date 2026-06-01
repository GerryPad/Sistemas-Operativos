#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAMANO_IR 64 //Tamaño de instruccion
#define INSTRUCCIONES_POR_MARCO 4
#define TAMANO_MARCO (TAMANO_IR * INSTRUCCIONES_POR_MARCO) 

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

//Estructura para representar un marco en memoria física (RAM)
struct MarcoPagina {
    char instrucciones[INSTRUCCIONES_POR_MARCO][TAMANO_IR];
};

void cargarMarcoDesdeBinario(const char *archivoBinario, int numeroMarco) {
    FILE *bin = fopen(archivoBinario, "rb");
    if (!bin) {
        printf("Error al abrir la memoria virtual.\n");
        return;
    }

    //Calcular la posición del marco
    long posicionBytes = (long)numeroMarco * TAMANO_MARCO;

    //Mover el puntero del archivo a esa posición
    if (fseek(bin, posicionBytes, SEEK_SET) != 0) {
        printf("Error: El marco %d no existe en la memoria virtual.\n", numeroMarco);
        fclose(bin);
        return;
    }

    //Crear un contenedor para el marco de la RAM
    struct MarcoPagina miMarcoRAM;

    //Leer las 4 instrucciones (256 bytes totales)
    size_t leidos = fread(&miMarcoRAM, sizeof(char), TAMANO_MARCO, bin);

    //Validar cuántas instrucciones reales pudimos leer (por si es el final del archivo)
    int instruccionesLeidas = leidos / TAMANO_IR;
    
    printf("\nContenido del Marco %d (%d bytes leídos) ---\n", numeroMarco, (int)leidos);
    for (int i = 0; i < instruccionesLeidas; i++) {
        printf("  Instruccion [%d] (Offset %d): %s\n", i + (numeroMarco * 4), i, miMarcoRAM.instrucciones[i]);
    }

    fclose(bin);
}

int main() {
    //Conversion de archivo de texto a binario 
    guardarTextoABinario("file7", "disco_virtual.bin");

    //Simulamos que el OS necesita cargar el Marco 1 y 2 a la RAM
    cargarMarcoDesdeBinario("disco_virtual.bin", 1); 
    cargarMarcoDesdeBinario("disco_virtual.bin", 2);

    return 0;
}