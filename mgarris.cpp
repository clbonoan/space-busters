//    |\   \\\\__     o
//    | \_/    o \    o 
//    > _   (( <_  oo  
//    | / \__+___/      
//    |/     |/

#include "fonts.h"
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <cstdio>
#include <cstring>

class Global {
    public:
        int xres, yres;
        char keys[65536];
    };

class Game {
public:
    enum GameMode {
        MAIN_MENU,
        ENDLESS_MODE,
        BOSS_MODE,
        SHIP_SELECTION,
        PAUSE_MENU
    };
    GameMode gameMode;
    bool inMenu;
    int selectedShip;
};

// Now declare extern so they reference the actual instances in space.cpp
extern Global gl;
extern Game g;
extern void setGameMode(Game::GameMode mode);
    
using namespace std;

int selectedShip = 0;

void show_mgarris(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Maxx - Also Here");
}

void render_ship_selection() {
    glClear(GL_COLOR_BUFFER_BIT);

    Rect r;
    r.bot = gl.yres - 90;
    r.left = gl.xres / 2 - 100;
    r.center = 0;
    ggprint8b(&r, 24, 0x00ffffff, "Select Your Ship");

    float centerX = gl.xres / 2;
    float centerY = gl.yres / 2; 
    float size = 50.0f;            
    float spacing = 160.0f;        

    // three options in middle
    for (int i = 0; i < 3; i++) {
        float xPos = centerX + (i - 1) * spacing;  
        float yPos = centerY;

        glPushMatrix();
        glTranslatef(xPos, yPos, 0);
        glColor3f(i == 0 ? 1.0f : 0.5f, i == 1 ? 1.0f : 0.5f, i == 2 ? 1.0f : 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(-size, -size);
        glVertex2f(-size, size);
        glVertex2f(size, size);
        glVertex2f(size, -size);
        glEnd();
        glPopMatrix();

        // hghlight ship
        if (selectedShip == i) {
            glColor3f(1.0f, 1.0f, 0.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(xPos - size - 5, yPos - size - 5);
            glVertex2f(xPos + size + 5, yPos - size - 5);
            glVertex2f(xPos + size + 5, yPos + size + 5);
            glVertex2f(xPos - size - 5, yPos + size + 5);
            glEnd();
        }
    }

    r.bot = 100;
    r.left = gl.xres / 2 - 120;
    ggprint8b(&r, 16, 0x00ffffff, "Use Left/Right to select");
    ggprint8b(&r, 16, 0x00ffffff, "Press Enter to confirm");
}

// ship selection
void handle_ship_selection_input() {

    if (gl.keys[XK_Left]) {
        selectedShip = (selectedShip - 1 + 3) % 3;  
        gl.keys[XK_Left] = 0;
    }
    if (gl.keys[XK_Right]) {
        selectedShip = (selectedShip + 1) % 3;  
        gl.keys[XK_Right] = 0;
    }

    if (gl.keys[XK_Return]) {
            g.selectedShip = selectedShip;  
            g.inMenu = true;  
            setGameMode(Game::MAIN_MENU);
    }
}
