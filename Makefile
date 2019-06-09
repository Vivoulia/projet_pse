# TP1 : Fichier Makefile
#
include ../Makefile.inc

EXE = serveur client data_function

all: ${EXE}

clean:
	rm -f *.o *~ ${EXE}


