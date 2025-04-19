//christine bonoan 
#include <iostream>
#include <fstream>
#include "fonts.h"
#include "cbonoan.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>

//extern Global gl;
//extern Game g;


using namespace std;

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
void drawHealthBar(int xres, float health)
{
    if (health > 1.0f)
        health = 1.0f;
    if (health < 0.0f)
        health = 0.0f;

    // position for health bar at bottom right corner
    float healthBarWidth = 200.0f;
    float healthBarHeight = 15.0f;
    float barX = xres - healthBarWidth - 20;     // shows 10px from the right
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
    glVertex2f(barX + (healthBarWidth * health), barY);
    glVertex2f(barX + (healthBarWidth * health), barY + healthBarHeight);
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

// -------------------------------------------------------------------
// add menu screen to choose endless mode or boss mode before starting
// -------------------------------------------------------------------
void drawMenu(int title, int xres, int yres, GLuint titleTexture, int menuSelection) 
{
    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);
    if (title) {
        glBindTexture(GL_TEXTURE_2D, titleTexture);
        //width and height for title
        int titleWidth = 692;
        int titleHeight = 47;
        int titlexStart = (xres - titleWidth) / 2;
        int titlexEnd = titlexStart + titleWidth;
        int titleyStart = yres - titleHeight - 300;
        int titleyEnd = yres - 300;
        /*glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(0, gl.yres);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(gl.xres, gl.yres);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(gl.xres, 0);
        glEnd();*/
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2i(titlexStart, titleyStart);
            glTexCoord2f(0.0f, 0.0f); glVertex2i(titlexStart, titleyEnd);
            glTexCoord2f(1.0f, 0.0f); glVertex2i(titlexEnd, titleyEnd);
            glTexCoord2f(1.0f, 1.0f); glVertex2i(titlexEnd, titleyStart);
        glEnd();
    }

    //glClear(GL_COLOR_BUFFER_BIT);
    r.bot = yres / 2;
    r.left = xres / 2 - 100;
    r.center = 0;
    glEnable(GL_TEXTURE_2D);

    // title
    //ggprint8b(&r, 24, 0x00ffff00, "Game Menu");

    // options
    ggprint8b(&r, 16, (menuSelection == 0) ? 0x00ff0000 : 0x00ffffff, "Endless Mode");
    ggprint8b(&r, 16, (menuSelection == 1) ? 0x00ff0000 : 0x00ffffff, "Boss Mode");
    ggprint8b(&r, 16, (menuSelection == 2) ? 0x00ff0000 : 0x00ffffff, "Ship Selection");
    ggprint8b(&r, 16, (menuSelection == 3) ? 0x00ff0000 : 0x00ffffff, "Exit");

}

//-----------------------------------------------------------------
// add pause menu
//-----------------------------------------------------------------
void drawPauseMenu(int xres, int yres, int menuSelection) 
{
    // draw "resume", "main menu", and "exit" options
    Rect r;
    r.bot = yres / 2;
    r.left = xres / 2 - 100;
    r.center = 0;

    ggprint8b(&r, 24, 0x00ffff00, "Pause Menu");
    ggprint8b(&r, 16, (menuSelection == 0) ? 0x00ff0000 : 0x00ffffff, "Resume");
    ggprint8b(&r, 16, (menuSelection == 1) ? 0x00ff0000 : 0x00ffffff, "Return to Main Menu");
    ggprint8b(&r, 16, (menuSelection == 2) ? 0x00ff0000 : 0x00ffffff, "Exit");

}


//--------------------------------------------------------------
//added function to handle main menu choices
//--------------------------------------------------------------
void handleMainMenuInput(char keys[], int &menuSelection, bool prevKeys[], 
        int &gameMode, bool &inMenu) {

    // handle menu navigation with keyboard input
    if (keys[XK_Up] && !prevKeys[XK_Up])
        menuSelection = (menuSelection == 0) ? 3 : menuSelection - 1;
    if (keys[XK_Down] && !prevKeys[XK_Down])
        menuSelection = (menuSelection == 3) ? 3 : menuSelection + 1;
    //g.menuSelection ^= 1;   //XOR to toggle between 0 and 1

    // update prevKeys; prevents multiple presses per key press
    memcpy(prevKeys, keys, 65536);

    // confirm selection with Enter key
    if (keys[XK_Return]) {
        inMenu = false;

        // start selected game mode
        switch (menuSelection) {
            case 0: // endless mode
                //gameMode = Game::ENDLESS_MODE;
                gameMode = 1;
                //gameMode = static_cast<int>(Game::ENDLESS_MODE);
                break;
            case 1: // boss mode
                //gameMode = static_cast<int>(Game::BOSS_MODE);
                gameMode = 2;
                //gameMode = Game::BOSS_MODE;
                break;
            case 2: // ship selection
                //gameMode = static_cast<int>(Game::SHIP_SELECTION);
                gameMode = 3;
                //gameMode = Game::SHIP_SELECTION;
                break;
            case 3: // exit
                exit(0);
                break;
        }
    }
}

// ---------------------------------------------------------
// add function to handle pause menu input
// ---------------------------------------------------------
void handlePauseMenuInput(char keys[], int &menuSelection, bool prevKeys[], 
        int &gameMode, bool &inMenu, bool &isPaused) {
    if (keys[XK_Up] && !prevKeys[XK_Up])
        menuSelection = (menuSelection == 0) ? 2 : menuSelection - 1;
    if (keys[XK_Down] && !prevKeys[XK_Down])
        menuSelection = (menuSelection == 2) ? 0 : menuSelection + 1;

    // update prevKeys; prevents multiple presses per key press
    memcpy(prevKeys, keys, 65536);

    // selecting option
    if (keys[XK_Return]) {
        isPaused = false;
        switch (menuSelection) {
            case 0: // resume
                isPaused = false;
                break;
            case 1: // go to main menu
                inMenu = true;
                isPaused = false;
                gameMode = 1;
                //gameMode = Game::MAIN_MENU;
                break;
            case 2: // exit game
                exit(0);
                break;
        }
    }
}

