# TP1 : Fichier Makefile
#
include ../Makefile.inc

EXE = serveur client

all: ${EXE} 

serveur: serveur.o data_function.o
	gcc -o $@ $^ $(LDLIBS)

data_function.o: data_function.c
	gcc -o $@ -c $< $(CFLAGS)

serveur.o: serveur.c data_function.h
	gcc -o $@ -c $< $(CFLAGS)
	
clean:
	rm -f *.o *~ ${EXE}






