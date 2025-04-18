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
const int MAX_ENEMIES = 30;
GLuint enemyTexture = 0;

struct Enemy {
    float x, y;
    float vx, vy;
    float ax, ay;
    float speed;
    bool active;
};

Enemy enemies[MAX_ENEMIES];
int enemySpawnTimer = 0;

extern unsigned char *buildAlphaData(Image *img);
extern float shipTargetPos[2];

void initEnemies() 
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }

    //load texture for enemy once
    glGenTextures(1, &enemyTexture);
    glBindTexture(GL_TEXTURE_2D, enemyTexture);

    Image img("./images/enemy1.png");
    printf("loaded enemy image: %d x %d\n", img.width, img.height);
    unsigned char *enemyData = buildAlphaData(&img);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
             0, GL_RGBA, GL_UNSIGNED_BYTE, enemyData);

    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
    //             0,GL_RGB, GL_UNSIGNED_BYTE, enemyData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(enemyData);
}

void spawnEnemy()
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x = rand() % gl.xres;
            enemies[i].y = gl.yres;
            enemies[i].vx = enemies[i].vy = 0.0f;
            enemies[i].ax = enemies[i].ay = 0.0f;
            enemies[i].speed = 0.5f;
            enemies[i].active = true;
            printf("spawned enemy at (%f, %f)\n", enemies[i].x, enemies[i].y);
            break;
        }
    }
}


void moveEnemiesTowardPlayer() 
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            //float dx = g.shipPosX - enemies[i].x;
            //float dy = g.shipPosY - enemies[i].y;
            //float shipX = g.ship.pos[0];
            //float shipY = g.ship.pos[1];
            //float dx = shipX - enemies[i].x;
            //float dy = shipY - enemies[i].y;
            float dx = shipTargetPos[0] - enemies[i].x;
            float dy = shipTargetPos[1] - enemies[i].y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist > 0.0f) {
                float accel = 0.25f;
                float damping = 0.90f;
                float maxSpeed = 1.8f;

                enemies[i].vx += (dx / dist) * accel;
                enemies[i].vy += (dy / dist) * accel;

                //Velocity and Acceleration
                enemies[i].vx *= damping;
                enemies[i].vy *= damping;
                
                //speed cap
                float speed = sqrt(enemies[i].vx * enemies[i].vx + 
                                   enemies[i].vy * enemies[i].vy);
                if (speed > maxSpeed) {
                    float scale = maxSpeed / speed;
                    enemies[i].vx *= scale;
                    enemies[i].vy *= scale;
                }

                enemies[i].x += enemies[i].vx;
                enemies[i].y += enemies[i].vy;

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
    glBindTexture(GL_TEXTURE_2D, enemyTexture);

    for (int i=0; i<MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            glPushMatrix();
            glTranslatef(enemies[i].x, enemies[i].y, 0);

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
   if (spawnTime >= 3600) {
       spawnEnemy();
       spawnTime = 0;
   }
} 

void hitEnemy(float x, float y) 
{
    for (int i=0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float dx = enemies[i].x - x;
            float dy = enemies[i].y - y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 30.0f) {
                enemies[i].active = false;
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
    Image &img = introImages[introFrame];

    // Bind texture
    GLuint introTexture;
    glGenTextures(1, &introTexture);
    glBindTexture(GL_TEXTURE_2D, introTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, 
                img.width, img.height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, img.data);

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
    //introFrame, frameCounter);

    // Update frame counter
    //frameCounter++;
    //if (frameCounter > 1) { // Change image every 50 frames
    //    introFrame = (introFrame + 1) % NUM_INTRO_FRAMES;
    //    frameCounter = 0;
    //}

    // End intro after one full cycle of images
    //if (introFrame == NUM_INTRO_FRAMES - 1 && frameCounter >= 3) {
    //    usleep(500000);
    //    //printf("Intro finished! Switching to Main Menu.\n");
    //    introDone = true; // Mark the intro as complete
    //    setGameMode(Game::MAIN_MENU); // Move to the menu
    //    /*
    //    // Ensure a final screen refresh happens
    //    glClear(GL_COLOR_BUFFER_BIT);
    //    glLoadIdentity();
    //    glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
    //    glFlush();
    //    */
    //}


    //usleep(20000); // Pause for smooth animation
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
