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
using namespace std;

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

// Image array for intro animation
const int NUM_INTRO_FRAMES = 4;
Image introImages[NUM_INTRO_FRAMES] = {
    "./images/stardust-health.png",
    "./images/ufo-G.png",
    "./images/ufo-B.png",
    "./images/ufo-R.png"
};

// Animation variables
int introFrame = 0;
bool introDone = false;
int frameCounter = 0; // Controls timing for switching frames

void show_davalos(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Dan is a member");
}


// Start of program animated intro
void drawIntro() {
    if (introDone) return;

    glClear(GL_COLOR_BUFFER_BIT);

    // Reset the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Get current image
    Image &currentImage = introImages[introFrame];

    // Bind texture
    GLuint introTexture;
    glGenTextures(1, &introTexture);
    glBindTexture(GL_TEXTURE_2D, introTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, 
                currentImage.width, currentImage.height, 0,
                GL_RGB, GL_UNSIGNED_BYTE, currentImage.data);

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
    frameCounter++;
    if (frameCounter > 3) { // Change image every 50 frames
        introFrame = (introFrame + 1) % NUM_INTRO_FRAMES;
        frameCounter = 0;
    }

    // End intro after one full cycle of images
    if (introFrame == NUM_INTRO_FRAMES - 1 && frameCounter >= 3) {
        usleep(500000);
        //printf("Intro finished! Switching to Main Menu.\n");
        introDone = true; // Mark the intro as complete
        setGameMode(Game::MAIN_MENU); // Move to the menu
        /*
        // Ensure a final screen refresh happens
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
        glFlush();
        */
    }


    usleep(20000); // Pause for smooth animation
}
