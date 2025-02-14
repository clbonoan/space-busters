CFLAGS = -I ./include
##LIB 	= ./libggfonts.so
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrander

all: asteroids

asteroids: asteroids.cpp log.cpp timers.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarios.cpp
	g++ $(CFLAGS) asteroids.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp log.cpp timers.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -o asteroids

clean:
	rm -f asteroids
	rm -f *.o
