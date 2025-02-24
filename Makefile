CFLAGS = -I ./include
##LIB 	= ./libggfonts.so
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrander

all: asteroids space

asteroids: asteroids.cpp log.cpp timers.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp mgarris.cpp
	g++ $(CFLAGS) asteroids.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp mgarris.cpp log.cpp timers.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -o asteroids

space: space.cpp log.cpp timers.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp mgarris.cpp
	g++ $(CFLAGS) space.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp mgarris.cpp log.cpp timers.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -o space
	
clean:
	rm -f asteroids
	rm -f space
	rm -f *.o
