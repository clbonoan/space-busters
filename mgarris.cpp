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
#include <vector>
#include <ctime>
#include <cmath>
#include <vector>

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
extern double timeDiff(struct timespec *, struct timespec *);

using namespace std;

void show_mgarris(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Maxx - Also Here");
}

// =================== PARTICLE SYSTEM ====================

struct Particle {
    float pos[2];
    float vel[2];
    float color[3];
    float life;
    float alpha;
};

vector<Particle> particles;

void addThrustParticle(float x, float y, float dx, float dy) {
    Particle p;
    p.pos[0] = x;
    p.pos[1] = y;
    p.vel[0] = dx + ((rand() % 100 - 50) / 100.0);
    p.vel[1] = dy + ((rand() % 100 - 50) / 100.0);
    p.life = 1.0;
    p.alpha = 1.0;

    p.color[0] = 1;
    p.color[1] = 1;
    p.color[2] = 1;

    particles.push_back(p);
}

void updateParticles() {
    for (size_t i = 0; i < particles.size(); ) {
        particles[i].pos[0] += particles[i].vel[0];
        particles[i].pos[1] += particles[i].vel[1];
        particles[i].life -= 0.02;
        particles[i].alpha = particles[i].life;
        if (particles[i].life <= 0.0) {
            particles.erase(particles.begin() + i);
        } else {
            ++i;
        }
    }
}

void renderParticles() {
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f);

    glBegin(GL_POINTS);
    for (const Particle& p : particles) {
        glColor4f(p.color[0], p.color[1], p.color[2], p.alpha);
        glVertex2f(p.pos[0], p.pos[1]);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

// ================= SHIP SELECTION =================
void render_ship_selection() {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(gl.xres / 2, gl.yres - 90, 0);
    glScalef(2.5f, 2.5f, 1.0f);
    Rect r;
    r.bot = 0;
    r.left = 0;
    r.center = 1;
    ggprint8b(&r, 48, 0x00ffffff, "Select Your Ship");
    glPopMatrix();

    glPushMatrix();
    glTranslatef(gl.xres / 2, 100, 0);
    glScalef(2.0f, 2.0f, 1.0f);
    r.bot = 0;
    r.left = 0;
    r.center = 1;
    ggprint8b(&r, 48, 0x00ffffff, "Use Left/Right to select");
    ggprint8b(&r, 48, 0x00ffffff, "Press Enter to confirm");
    glPopMatrix();

    float centerX = gl.xres / 2;
    float centerY = gl.yres / 2;
    float size = 80.0f;
    float spacing = 200.0;

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
}

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
