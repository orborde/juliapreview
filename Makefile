CC=gcc
CFLAGS= -O3 -Werror
LDFLAGS=
BINFLAGS= $(CFLAGS) $(LDFLAGS)

all : juliapreview juliapreview2

clean :
	rm complex.o

complex.o : complex.h complex.c
	$(CC) $(CFLAGS) -c complex.c

juliapreview : complex.o juliapreview.c
	$(CC) $(BINFLAGS) complex.o juliapreview.c -lSDL -o juliapreview

juliapreview2 : complex.o juliapreview2.c
	$(CC) $(BINFLAGS) complex.o juliapreview2.c -lSDL -o juliapreview2
