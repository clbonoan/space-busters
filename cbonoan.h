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
void show_christine(Rect *r);
void show_credits(Rect *r);
void drawHealthBar(int xres, float health);
void drawMenu(int title, int xres, int yres, GLuint titleTexture, int menuSelection);
void drawPauseMenu(int xres, int yres, int menuSelection);
//void handleMainMenuInput();
void handleMainMenuInput(char keys[], int &menuSelection, bool prevKeys[], 
        int &gameMode, bool &inMenu);
void handlePauseMenuInput(char keys[], int &menuSelection, bool prevKeys[], 
        int &gameMode, bool &inMenu, bool &isPaused);
void screenLeftText(int yres);
//void screenRightText(int xres, int yres, int score);
void show_instructions(Rect *r);
#endif


