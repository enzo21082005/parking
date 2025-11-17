CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11
LDFLAGS := -lncursesw -lrt  # Ajout de -lrt pour nanosleep

SRC     := main.c affichage.c gestion.c
OBJ     := $(SRC:.c=.o)
EXEC    := parking

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)  

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f $(OBJ) $(EXEC) 

.PHONY: all clean