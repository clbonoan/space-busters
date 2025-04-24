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

// made changes to space file
// changed asteroids and fragments to circles;
// updated background to make more space like
// figuring out how to upload image to change into background
// added functionality for ship thrust with down arrow like up arrow
//
//

