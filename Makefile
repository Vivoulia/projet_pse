# TP1 : Fichier Makefile
#
include ../Makefile.inc

EXE = serveur client

all: ${EXE} 
	gcc -c data_function.c

clean:
	rm -f *.o *~ ${EXE}






