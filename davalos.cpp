/*   ___~~~___
    /  /_ _\  \
   (__/)O_O(\__)      
----(u)\(_)/(u)---
___|____|____|____|

8===8

*/

#include "fonts.h"
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

using namespace std;

class Global {
    public:
        int xres, yres;
        char keys[65536];
};

class Ship {
public:
    float pos[3];  // Only need pos to follow player
};

class Game {
    public:
        Ship ship;
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

class Image {
    public:
        int width, height;
        unsigned char *data;
        ~Image() { delete [] data; }
        Image(const char *fname);
};

// Declare external images
extern Image img[];


extern Global gl;
extern Game g;
extern void setGameMode(Game::GameMode mode);

//Enemy logic
const int MAX_ENEMIES = 100;
GLuint zorpTexture = 0;
GLuint wiblobTexture = 0;

struct Enemy {
    float x, y;
    float vx, vy;
    float ax, ay;
    float speed;
    bool active;
};

Enemy zorpArmy[MAX_ENEMIES];
Enemy wiblobArmy[MAX_ENEMIES];
GLuint goldenEnemyTexture = 0;
int enemySpawnTimer = 0;

extern unsigned char *buildAlphaData(Image *img);
extern float shipTargetPos[2];

void initEnemies() 
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        //green alien army
        zorpArmy[i].active = 0;
        //gold alien army
        wiblobArmy[i].active = 0;
    }

    //load texture for enemies
    
    //green alien
    glGenTextures(1, &zorpTexture);
    glBindTexture(GL_TEXTURE_2D, zorpTexture);

    Image zorpImg("./images/zorp.png");
    //printf("loaded Zorp image: %d x %d\n", zorpImg.width, zorpImg.height);
    unsigned char *zorpData = buildAlphaData(&zorpImg);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, zorpImg.width, zorpImg.height,
             0, GL_RGBA, GL_UNSIGNED_BYTE, zorpData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(zorpData);

    //gold alien
    glGenTextures(1, &wiblobTexture);
    glBindTexture(GL_TEXTURE_2D, wiblobTexture);

    Image wiblobImg("./images/wiblob.png");
    unsigned char *wiblobData = buildAlphaData(&wiblobImg);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wiblobImg.width,
                 wiblobImg.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, wiblobData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    free(wiblobData);
}

void spawnEnemy()
{
    int type = rand() % 2;
    
    //spawns the zorp army
    if( type != 1) {
        for (int i=0; i < MAX_ENEMIES; i++) {
            if (!zorpArmy[i].active) {
                zorpArmy[i].x = rand() % gl.xres;
                zorpArmy[i].y = gl.yres;
                zorpArmy[i].vx = zorpArmy[i].vy = 0.0f;
                zorpArmy[i].ax = zorpArmy[i].ay = 0.0f;
                zorpArmy[i].speed = 0.5f;
                zorpArmy[i].active = true;
                //printf("spawned zorp at (%f, %f)\n",
                //        zorpArmy[i].x, zorpArmy[i].y);
                break;
            }
        }
    }
    //spawns the wiblob army
    if (type == 1) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!wiblobArmy[i].active) {
                wiblobArmy[i].x = rand() % gl.xres;
                wiblobArmy[i].y = gl.yres + 50;
                wiblobArmy[i].vx = wiblobArmy[i].vy = 0.0f;
                wiblobArmy[i].ax = wiblobArmy[i].ay = 0.0f;
                wiblobArmy[i].speed = 0.5f;
                wiblobArmy[i].active = true;
                //printf("spawned wiblob at (%f, %f)\n",
                //        wiblobArmy[i].x, wiblobArmy[i].y);
                break;
            }
        }
    }
}


void moveEnemiesTowardPlayer() 
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        if (zorpArmy[i].active) {
            float dx = shipTargetPos[0] - zorpArmy[i].x;
            float dy = shipTargetPos[1] - zorpArmy[i].y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist > 0.0f) {
                float accel = 0.25f;
                float damping = 0.90f;
                float maxSpeed = 1.8f;

                zorpArmy[i].vx += (dx / dist) * accel;
                zorpArmy[i].vy += (dy / dist) * accel;

                //Velocity and Acceleration
                zorpArmy[i].vx *= damping;
                zorpArmy[i].vy *= damping;
                
                //speed cap
                float speed = sqrt(zorpArmy[i].vx * zorpArmy[i].vx + 
                                   zorpArmy[i].vy * zorpArmy[i].vy);
                if (speed > maxSpeed) {
                    float scale = maxSpeed / speed;
                    zorpArmy[i].vx *= scale;
                    zorpArmy[i].vy *= scale;
                }

                zorpArmy[i].x += zorpArmy[i].vx;
                zorpArmy[i].y += zorpArmy[i].vy;

            }
        }
        if (wiblobArmy[i].active) {
            float dx = shipTargetPos[0] - wiblobArmy[i].x;
            float dy = shipTargetPos[1] - wiblobArmy[i].y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist > 0.0f) {
                float accel = 0.25f;
                float damping = 0.90f;
                float maxSpeed = 1.8f;

                wiblobArmy[i].vx += (dx / dist) * accel;
                wiblobArmy[i].vy += (dy / dist) * accel;

                //Velocity and Acceleration
                wiblobArmy[i].vx *= damping;
                wiblobArmy[i].vy *= damping;

                //speed cap
                float speed = sqrt(wiblobArmy[i].vx *
                                    wiblobArmy[i].vx +
                                    wiblobArmy[i].vy *
                                    wiblobArmy[i].vy);
                if (speed > maxSpeed) {
                    float scale = maxSpeed / speed;
                    wiblobArmy[i].vx *= scale;
                    wiblobArmy[i].vy *= scale;
                }

                wiblobArmy[i].x += wiblobArmy[i].vx;
                wiblobArmy[i].y += wiblobArmy[i].vy;
            }
        }
    }
}

void renderEnemies()
{
    float w = 32.0f, h = 32.0f;
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, zorpTexture);

    for (int i=0; i<MAX_ENEMIES; i++) {
        if (zorpArmy[i].active) {
            glPushMatrix();
            glTranslatef(zorpArmy[i].x, zorpArmy[i].y, 0);

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(-w, -h);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(-w, h);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(w, h);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(w, -h);
            glEnd();
            glPopMatrix();
        }
    }

    glBindTexture(GL_TEXTURE_2D, wiblobTexture);

    for (int i=0; i<MAX_ENEMIES; i++) {
        if (wiblobArmy[i].active) {
            //printf("Rendering wiblob at (%f, %f)\n",
            //       wiblobArmy[i].x, wiblobArmy[i].y);
            glPushMatrix();
            glTranslatef(wiblobArmy[i].x, wiblobArmy[i].y, 0);

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(-w, -h);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(-w, h);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(w, h);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(w, -h);
            glEnd();
            glPopMatrix();
        }
    }
 
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void updateEnemySpawnTimer() 
{
   static int spawnTime = 0;
   spawnTime++;
   if (spawnTime >= 20) {
       spawnEnemy();
       spawnTime = 0;
   }
   //printf("Enemy timer tick: %d\n", spawnTime);
} 

void hitEnemy(float x, float y) 
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        if (zorpArmy[i].active) {
            float dx = zorpArmy[i].x - x;
            float dy = zorpArmy[i].y - y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 30.0f) {
                zorpArmy[i].active = false;
                break;
            }
        }
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (wiblobArmy[i].active) {
            float dx = wiblobArmy[i].x - x;
            float dy = wiblobArmy[i].y - y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < 30.0f) {
                wiblobArmy[i].active = false;
                //printf("golden enemy hit at (%f, %f)\n",
                //       wiblobArmy[i].x, wiblobArmy[i].y);
                break;
            }
        }
    }
}

// Animation variables
int introFrame = 0;
bool introDone = false;
int frameCounter = 0;
int nebulaFrame = 0;
int nebulaCounter = 0;

// Image array for intro animation
const int NUM_INTRO_FRAMES = 36;

Image introImages[NUM_INTRO_FRAMES] = {
    "./images/apple_frames/intro0.png",  "./images/apple_frames/intro1.png",
    "./images/apple_frames/intro2.png",  "./images/apple_frames/intro3.png",
    "./images/apple_frames/intro4.png",  "./images/apple_frames/intro5.png",
    "./images/apple_frames/intro6.png",  "./images/apple_frames/intro7.png",
    "./images/apple_frames/intro8.png",  "./images/apple_frames/intro9.png",
    "./images/apple_frames/intro10.png", "./images/apple_frames/intro11.png",
    "./images/apple_frames/intro12.png", "./images/apple_frames/intro13.png",
    "./images/apple_frames/intro14.png", "./images/apple_frames/intro15.png",
    "./images/apple_frames/intro16.png", "./images/apple_frames/intro17.png",
    "./images/apple_frames/intro18.png", "./images/apple_frames/intro19.png",
    "./images/apple_frames/intro20.png", "./images/apple_frames/intro21.png",
    "./images/apple_frames/intro22.png", "./images/apple_frames/intro23.png",
    "./images/apple_frames/intro24.png", "./images/apple_frames/intro25.png",
    "./images/apple_frames/intro26.png", "./images/apple_frames/intro27.png",
    "./images/apple_frames/intro28.png", "./images/apple_frames/intro29.png",
    "./images/apple_frames/intro30.png", "./images/apple_frames/intro31.png",
    "./images/apple_frames/intro32.png", "./images/apple_frames/intro33.png",
    "./images/apple_frames/intro34.png", "./images/apple_frames/intro35.png"
};


// Image array for nebula background animation
const int NUM_NEBULA_FRAMES = 3;
Image nebulaImages[NUM_NEBULA_FRAMES] = {
    "./images/neb1.png",
    "./images/neb2.png",
    "./images/neb3.png"
};

void show_davalos(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Dan is a member");
}


// Start of program animated intro
void drawIntro() 
{
    if (introDone) return;

    glClear(GL_COLOR_BUFFER_BIT);

    // Reset the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Get current image
    Image &introImg = introImages[introFrame];

    // Bind texture
    GLuint introTexture;
    glGenTextures(1, &introTexture);
    glBindTexture(GL_TEXTURE_2D, introTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, 
                introImg.width, introImg.height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, introImg.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_TEXTURE_2D);

    // Draw fullscreen quad with correct scaling
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, gl.yres);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(gl.xres, gl.yres);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(gl.xres, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &introTexture);

    //debugging
    //printf("intro frame: %d, frame counter: %d\n", 

    //smooth transition
    usleep(6000);
    introFrame++;

    if (introFrame >= NUM_INTRO_FRAMES) {
        introDone = true;
        setGameMode(Game::MAIN_MENU);
    }
}

void drawNebulaBackground() 
{
    Image &currentFrame = nebulaImages[nebulaFrame];
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, currentFrame.width, currentFrame.height,
                0, GL_RGB, GL_UNSIGNED_BYTE, currentFrame.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(0, gl.yres);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(gl.xres, gl.yres);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(gl.xres, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &tex);

    nebulaCounter++;
    if (nebulaCounter > 5) {
        nebulaFrame = (nebulaFrame + 1) % NUM_NEBULA_FRAMES;
        nebulaCounter = 0;
    }
}
