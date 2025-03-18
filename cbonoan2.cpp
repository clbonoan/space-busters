//christine bonoan 
#include <iostream>
#include <fstream>
#include "fonts.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

class Global {
public:
    int xres, yres;
};

class Ship {
public:
    float health;

    Ship() {
        health = 1.0f;
    }
};

class Game {
public:
    Ship ship;
//    GLuint stardustTexture;
//    GLuint ufoTexture;
//    int showStardust;
};

extern Global gl;
extern Game g;

using namespace std;

//ufo variables
//GLuint ufoTexture;
//int ufoWidth, ufoHeight;

void show_christine(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Christine - Unknown");
}

void show_instructions(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "WASD to move");
    ggprint8b(r, 16, 0x00ff00ff, "Shift to boost");
    ggprint8b(r, 16, 0x00ff00ff, "Q to turn left");
    ggprint8b(r, 16, 0x00ff00ff, "E to turn right");
    ggprint8b(r, 16, 0x00ff00ff, "Space to shoot");
}

// --------------------------------------------------------------------
// added health bar function
// --------------------------------------------------------------------
void drawHealthBar(float health)
{
    if (health > 1.0f)
        health = 1.0f;
    if (health < 0.0f)
        health = 0.0f;

    // position for health bar at bottom right corner
    float healthBarWidth = 200.0f;
    float healthBarHeight = 15.0f;
    float barX = gl.xres - healthBarWidth - 20;     // shows 10px from the right
    float barY = 10;    // shows 10px from the bottom

    Rect r;
    r.bot = barY + healthBarHeight + 5;
    r.left = barX;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ffff00, "HEALTH");

    //debug
    //printf("health: %f\n", g.ship.health);
    
    // draws red health background bar
    glColor3f(1.0f, 0.0f, 0.0f);    
    glBegin(GL_QUADS);
    glVertex2f(barX, barY);
    glVertex2f(barX + healthBarWidth, barY);
    glVertex2f(barX + healthBarWidth, barY + healthBarHeight);
    glVertex2f(barX, barY + healthBarHeight);
    glEnd();

    // draws blue health bar based on current ship health
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(barX, barY);
    glVertex2f(barX + (healthBarWidth * g.ship.health), barY);
    glVertex2f(barX + (healthBarWidth * g.ship.health), barY + healthBarHeight);
    glVertex2f(barX, barY + healthBarHeight);
    glEnd();

    // draws black lines to represent increments of health lost
    glColor3f(0.0f, 0.0f, 0.0f);
    float lineSpacing = healthBarWidth / 10.0f;
    for (int i = 1; i < 10; i++) {
        float xPos = barX + i * lineSpacing;
        glBegin(GL_LINES);
        glVertex2f(xPos, barY);
        glVertex2f(xPos, barY + healthBarHeight);
        glEnd();
    }
}


