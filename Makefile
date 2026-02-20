# Nombre del ejecutable final
TARGET = interprete

# Compilador a usar
CC = gcc

# Banderas de compilación 
# -Wall: muestra todas las advertencias 
# -g: añade información de depuración
CFLAGS = -Wall -g

# Archivos fuente (.c) y objetos (.o)
SRCS = main.c instrucciones.c
OBJS = $(SRCS:.c=.o)

# Regla principal: Compilar el proyecto completo
all: $(TARGET)

# Cómo crear el ejecutable a partir de los archivos objeto
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Cómo compilar cada archivo .c a un .o
# El main.c y instrucciones.c dependen de instrucciones.h
%.o: %.c instrucciones.h
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los archivos basura generados
clean:
	rm -f $(TARGET) $(OBJS)

# Regla para compilar y ejecutar de una vez
run: all
	./$(TARGET)