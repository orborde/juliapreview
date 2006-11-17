CC=gcc
CFLAGS= -O3 -Werror
LDFLAGS=
BINFLAGS= $(CFLAGS) $(LDFLAGS)

all : juliapreview juliapreview2

clean :
	echo I do nothing

juliapreview : complex.h juliapreview.c
	$(CC) $(BINFLAGS) juliapreview.c -lSDL -o juliapreview

juliapreview2 : complex.h juliapreview2.c
	$(CC) $(BINFLAGS) juliapreview2.c -lSDL -o juliapreview2
