#ifndef _CBONOAN_H_
#define _CBONOAN_H_
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
void drawHealthBar(int xres, float health);
void drawMenu(int title, int xres, int yres, GLuint titleTexture, int menuSelection);
void drawPauseMenu(int xres, int yres, int menuSelection);
//void handleMainMenuInput(char *keys, int menuSelection, bool *prevKeys, 
//        GameMode &gameMode, bool &inMenu);

#endif


