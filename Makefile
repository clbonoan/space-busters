CFLAGS = -I ./include
##LIB 	= ./libggfonts.so
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrander

all: space

space: space.cpp log.cpp timers.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp mgarris.cpp
	g++ $(CFLAGS) space.cpp cbonoan2.cpp eaviles.cpp davalos.cpp bbarrios.cpp mgarris.cpp log.cpp timers.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -o space
	
clean:
	rm -f space
	rm -f *.o
