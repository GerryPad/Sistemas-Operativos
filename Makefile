
TARGET = interprete

CC = gcc

# -Wall muestra todas las advertencias y -g añade información de depuración
# -lncurses es para poder compialr con ncurses
CFLAGS = -Wall -g
LDFLAGS = -lncurses

SRCS = main.c instrucciones.c ncurses.c nodo.c dispatch.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c instrucciones.h ncurses.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: all
	./$(TARGET)