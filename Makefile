#
# Makefile for heap example - process scheduler
#

CC=gcc
TARGET=ring
LIBS= -lm -Wall

SRCFILES= main.c Servidor.c Interface.c Command.c aux.c
INCFILES= Servidor.h Interface.h Command.h aux.h
OBJFILES= main.o Servidor.o Interface.o Command.o aux.o

default: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LIBS)

clean:
	rm -f $(TARGET) $(OBJFILES) core.* #*

aux.o: aux.h aux.c

Servidor.o: Servidor.h Servidor.c

Interface.o: Interface.h Interface.c

Command.o: Command.h Command.c

main.o: main.c Servidor.h Interface.h Command.h aux.h

VALG = valgrind --leak-check=full --show-leak-kinds=all

run1:
		for F in ${FILES1}; do ./ring $${F}; done;

vrun1:
		for F in ${FILES1}; do ${VALG} ./ring $${F}; done;
