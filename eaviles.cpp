//edwin aviles

#include <iostream>
#include <fstream>
#include "fonts.h"
#include "eaviles.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <cstring>
#include <cmath>



class Global {
    public:
        int xres, yres;
        char keys[65537];
};


class Image {
    public:
        int width, height;
        unsigned char *data;
        ~Image() {delete [] data; }
        Image(const char *fname);
};

extern Global gl;
extern Image img[];
extern unsigned char *buildAlphaData(Image *img);

HPpack HealObject[HealthPack];

GLuint healTexture = 0;

void initStar() 
{
    for (int i=0; i < HealthPack; i++) {
        
        HealObject[i].active = 0;
       
    }   

    //load texture for enemies
    
    //green alien
    glGenTextures(1, &healTexture);
    glBindTexture(GL_TEXTURE_2D, healTexture);

    Image starImg("./images/healObject.png");
   // printf("loaded Zorp image: %d x %d\n", zorpImg.width, zorpImg.height);
    unsigned char *starData = buildAlphaData(&starImg);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, starImg.width, starImg.height,
             0, GL_RGBA, GL_UNSIGNED_BYTE, starData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(starData);


}

void screenRightText(int xres, int yres, int score)
{
    Rect r;
    r.bot = yres - 20;
    r.left = xres - 100;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ffffff, "SCORE: %d", score);
    r.left = 10;
    r.bot = yres - 60;
}

void show_edwin(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Edwin - Lead Programmer");
}

//void Collision(float w, float x, float y, float z)
//{
  //  if(enemies[i].active) {
        




//}

void spawnStar()
{
    int type = rand() % 2;
    
    
    if( type != 1) {
        for (int i=0; i < HealthPack; i++) {
            if (!HealObject[i].active) {
                HealObject[i].x = rand() % gl.xres;
                HealObject[i].y = rand() % gl.yres;
                HealObject[i].active = true;
                printf("spawned star at (%f, %f)\n",
                        HealObject[i].x, HealObject[i].y);
                break;
            }   
        }   
    }
}

void captureStar(float x, float y)
{
    for( int i=0; i<HealthPack; i++) {
        if(HealObject[i].active) {
            float dx = HealObject[i].x - x;
            float dy = HealObject[i].y - y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 30.0f) {
                HealObject[i].active = false;
                break;
            }
        }
    }
}


void renderHeal()
{
    float w = 32.0f, h = 32.0f;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, healTexture);

    for (int i=0; i<HealthPack; i++) {
        if (HealObject[i].active) {
            glPushMatrix();
            glTranslatef(HealObject[i].x, HealObject[i].y, 0); 

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

void updateStarSpawnTimer()
{
    static int spawnTime = 0;
    spawnTime++;
    if (spawnTime >= 20) {
       spawnStar();
       spawnTime = 0;
    }
}


// made changes to space file
// changed asteroids and fragments to circles;
// updated background to make more space like
// figuring out how to upload image to change into background
// added functionality for ship thrust with down arrow like up arrow
//
//

