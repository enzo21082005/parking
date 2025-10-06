CC=gcc
CFLAGS=-Wall -Wextra

all: parking
parking: main.o affichage.o gestion.o
	$(CC) $(CFLAGS) -o parking main.o affichage.o gestion.o

main.o: main.c affichage.h gestion.h
affichage.o: affichage.c affichage.h
gestion.o: gestion.c gestion.h

clean:
	rm -f *.o parking
	
