#ifndef _EAVILESH
#define _EAVILESH
#include <GL/gl.h>
#include <GL/glu.h>
#include <X11/keysym.h>
#include <cstring>

const int HealthPack = 5;

struct HPpack {
    float x, y;
    bool active;
};

// forward classes from space.cpp
//class Global;
//class Game;

// declare global variables from space.cpp
//extern Global gl;
//extern Game g;
extern GLuint healTexture;
extern HPpack HealObject[HealthPack];

// prototypes
void spawnStar();
void screenRightText(int xres, int yres, int score);
void initStar();
void renderHeal();
void updateStarSpawnTimer();
void captureStar(float x, float y);
#endif
