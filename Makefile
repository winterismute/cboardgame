#Make
CC	= gcc
CFLAGS = -c -Wall -O2
LDLIB = -L/usr/X11R6/lib -lX11 -lSDL -lSDL_ttf -lSDL_image -lm

all:	clean othellogui

othellogui: othellogui.o GCGraLib.o
	$(CC) othellogui.o GCGraLib.o $(LDLIB) -o $@

othellogui.o: othellogui.c GCGraLib.c GCGraLib.h
	$(CC) $(CFLAGS) othellogui.c

GCGraLib.o: GCGraLib.c GCGraLib.h
	$(CC) $(CFLAGS) GCGraLib.c

clean:
	rm -f *.o *.*~ *~ othellogui 
