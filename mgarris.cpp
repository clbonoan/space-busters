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
};

extern Global gl;
extern Game g;
extern void setGameMode(Game::GameMode mode);
extern GLuint shipTextures[3];
extern int selectedShip;
extern void drawUFO(float x, float y, GLuint texture);
    
using namespace std;

void show_mgarris(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Maxx - Also Here");
}

void render_ship_selection() {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    Rect r;
    r.bot = gl.yres - 90;
    r.left = gl.xres / 2 - 100;
    r.center = 0;
    ggprint8b(&r, 24, 0x00ffffff, "Select Your Ship");


    
    float centerX = gl.xres / 2;
    float centerY = gl.yres / 2;
    float size = 80.0f;
    float spacing = 200.0f;



    for (int i = 0; i < 3; i++) {
        float xPos = centerX + (i - 1) * spacing;
        float yPos = centerY;
        drawUFO(xPos, yPos, shipTextures[i]);

        glPushMatrix();
        glTranslatef(xPos, yPos, 0);
        glScalef(1.0f, -1.0f, 1.0f);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, shipTextures[i]);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.1f);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-size, -size);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-size, size);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(size, size);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(size, -size);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_ALPHA_TEST);
        glPopMatrix();

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

    glEnable(GL_TEXTURE_2D);
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
            selectedShip = selectedShip; 
            g.inMenu = true;
            setGameMode(Game::MAIN_MENU);
    }
}
