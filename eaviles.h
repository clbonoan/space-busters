#ifndef _EAVILESH
#define _EAVILESH
#include <GL/gl.h>
#include <GL/glu.h>
#include <X11/keysym.h>
#include <cstring>


// forward classes from space.cpp
class Global;
class Game;

// declare global variables from space.cpp
extern Global gl;
extern Game g;

// prototypes
void screenRightText(int xres, int yres, int score);

#endif
