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

extern void show_davalos(Rect *r);
extern void show_edwin(Rect *r);
extern void show_bbarrios(Rect *r);
extern void show_mgarris(Rect *r);
extern void restartGame();

using namespace std;

void show_christine(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Christine - Unknown");
    printf("christine credit called\n");
}

void show_credits(Rect *r)
{
    show_christine(r);
    show_davalos(r);
    show_edwin(r);
    show_bbarrios(r);
    show_mgarris(r);
}

void show_instructions(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "WASD to move");
    ggprint8b(r, 16, 0x00ff00ff, "Shift to boost");
    ggprint8b(r, 16, 0x00ff00ff, "Q to turn left");
    ggprint8b(r, 16, 0x00ff00ff, "E to turn right");
    ggprint8b(r, 16, 0x00ff00ff, "Space to shoot");
}

void screenLeftText(int yres) 
{
    Rect r;
    r.bot = yres - 20;
    r.left = 10;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ffff00, "Press c for Credits");
    ggprint8b(&r, 16, 0x00ffff00, "Press i for Instructions");
}

// added health bar function
void drawHealthBar(int xres, float health)
{
    // set caps/bounds for health 
    if (health > 1.0f)
        health = 1.0f;
    if (health < 0.0f)
        health = 0.0f;

    // position for health bar at bottom right corner
    float healthBarWidth = 200.0f;
    float healthBarHeight = 15.0f;
    float barX = xres - healthBarWidth - 20;     
    float barY = 10;    

    Rect r;
    r.bot = barY + healthBarHeight + 5;
    r.left = barX;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ffff00, "HEALTH");

    // draws red health background bar
    glColor3f(1.0f, 0.0f, 0.0f);    
    glBegin(GL_QUADS);
    glVertex2f(barX, barY);
    glVertex2f(barX + healthBarWidth, barY);
    glVertex2f(barX + healthBarWidth, barY + healthBarHeight);
    glVertex2f(barX, barY + healthBarHeight);
    glEnd();
    
    // draws blue health bar based on current ship health
    glColor3f(0.0f, 1.0f, 0.0f);
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

// add menu screen to choose endless mode or boss mode before starting
void drawMenu(int title, int xres, int yres, GLuint titleTexture, 
        int menuSelection)
{
    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);

    if (title) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, titleTexture);
        int titleWidth = 692;
        int titleHeight = 47;
        int titlexStart = (xres - titleWidth) / 2;
        int titlexEnd = titlexStart + titleWidth;
        int titleyStart = yres - titleHeight - 200;
        int titleyEnd = yres - 200;
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); 
            glVertex2i(titlexStart, titleyStart);
            glTexCoord2f(0.0f, 0.0f); 
            glVertex2i(titlexStart, titleyEnd);
            glTexCoord2f(1.0f, 0.0f); 
            glVertex2i(titlexEnd, titleyEnd);
            glTexCoord2f(1.0f, 1.0f); 
            glVertex2i(titlexEnd, titleyStart);
        glEnd();
    }

    r.bot = yres / 2 + 20;
    r.left = xres / 2;
    r.center = 1;

    const int numOptions = 4;
    const char* options[numOptions] = {
        "Endless Mode", "Boss Mode", "Choose Ship", "Exit"
    };
    
    for (int i = 0; i < numOptions; i++) {
        ggprint16(&r, 32, (menuSelection == i) ? 0x00ff0000 : 0x00ffffff,
                options[i]);
    }
}

void drawGameOver(int gameOver, int xres, int yres, GLuint gameOverTexture,
        int menuSelection)
{
   Rect r;
   glClear(GL_COLOR_BUFFER_BIT);

   if (gameOver) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, gameOverTexture);
        int gameOverWidth = 775;
        int gameOverHeight = 77;
        int gameOverXStart = (xres - gameOverWidth) / 2;
        int gameOverXEnd = gameOverXStart + gameOverWidth;
        int gameOverYStart = yres - gameOverHeight - 200;
        int gameOverYEnd = yres - 200;
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2i(gameOverXStart, gameOverYStart);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2i(gameOverXStart, gameOverYEnd);
            glTexCoord2f(1.0f, 0.0f);
            glVertex2i(gameOverXEnd, gameOverYEnd);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2i(gameOverXEnd, gameOverYStart);
        glEnd();
   }

   r.left = xres / 2;
   r.bot = yres / 2 + 20;
   r.center = 1;
   glEnable(GL_TEXTURE_2D);

   // options
   ggprint16(&r, 32, (menuSelection == 0) ? 0x00ff0000 : 0x00ffffff, 
           "Restart");
   ggprint16(&r, 32, (menuSelection == 1) ? 0x00ff0000 : 0x00ffffff, 
           "Return to Main Menu");
   ggprint16(&r, 32, (menuSelection == 2) ? 0x00ff0000 : 0x00ffffff,
           "Exit");
}

// add pause menu
void drawPauseMenu(int xres, int yres, int menuSelection) 
{
    // draw "resume", "main menu", and "exit" options
    Rect r;
    r.bot = yres / 2 + 40;
    r.left = xres / 2;
    r.center = 1;
    glEnable(GL_TEXTURE_2D);

    ggprint16(&r, 32, 0x00ffff00, "Pause Menu");
    ggprint16(&r, 24, (menuSelection == 0) ? 0x00ff0000 : 0x00ffffff, 
            "Resume");
    ggprint16(&r, 24, (menuSelection == 1) ? 0x00ff0000 : 0x00ffffff, 
            "Return to Main Menu");
    ggprint16(&r, 24, (menuSelection == 2) ? 0x00ff0000 : 0x00ffffff, 
            "Exit");

}

//--------------------------------------------------------------
//added function to handle main menu choices
//--------------------------------------------------------------
void handleMainMenuInput(char keys[], int &menuSelection, bool prevKeys[], 
        int &gameMode, bool &inMenu) 
{
    // handle menu navigation with keyboard input
    if (keys[XK_Up] && !prevKeys[XK_Up])
        menuSelection = (menuSelection == 0) ? 3 : menuSelection - 1;
    if (keys[XK_Down] && !prevKeys[XK_Down])
        menuSelection = (menuSelection == 3) ? 0 : menuSelection + 1;
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
                restartGame();
                break;
            case 1: // boss mode
                //gameMode = Game::BOSS_MODE;               
                gameMode = 2;
                restartGame();
                break;
            case 2: // ship selection
                //gameMode = Game::SHIP_SELECTION;
                gameMode = 3;
                break;
            case 3: // exit
                exit(0);
                break;
        }
    }
}

// add function to handle pause menu input
void handlePauseMenuInput(char keys[], int &menuSelection, bool prevKeys[], 
        int &gameMode, bool &inMenu, bool &isPaused) 
{
    if (keys[XK_Up] && !prevKeys[XK_Up])
        menuSelection = (menuSelection == 0) ? 2 : menuSelection - 1;
    if (keys[XK_Down] && !prevKeys[XK_Down])
        menuSelection = (menuSelection == 2) ? 0 : menuSelection + 1;

    // update prevKeys; prevents multiple presses per key press
    memcpy(prevKeys, keys, 65536);

    // selecting option
    if (keys[XK_Return]) {
        //isPaused = false;
        isPaused = true;
        switch (menuSelection) {
            case 0: // resume
                isPaused = false;
                gameMode = gameMode;
                break;
            case 1: // go to main menu
                inMenu = true;
                isPaused = false;
                gameMode = 1;
                break;
            case 2: // exit game
                exit(0);
                break;
        }
    }
}

// add function to handle game over menu input
void handleGameOverInput(char keys[], int &menuSelection, bool prevKeys[],
        int &gameMode, bool &inMenu, bool &isPaused, bool &isEnd, 
        int &gameOver, bool &restartRequested)
{
    if (keys[XK_Up] && !prevKeys[XK_Up])
        menuSelection = (menuSelection == 0) ? 2 : menuSelection - 1;
    if (keys[XK_Down] && !prevKeys[XK_Down])
        menuSelection = (menuSelection == 2) ? 0 : menuSelection + 1;

    memcpy(prevKeys, keys, 65536);

    if (keys[XK_Return]) {
        switch (menuSelection) {
            case 0: // restart
                restartRequested = true;
                gameMode = gameMode;
                isEnd = false;
                gameOver = 0;
                break; 
            case 1: // go to main menu
                inMenu = true;
                isPaused = false;
                isEnd = false;
                gameOver = 0;
                gameMode = 0;
                break;
            case 2: // exit game
                exit(0);
                break;
        }
    }
}
