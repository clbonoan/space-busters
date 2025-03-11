#include <iostream>
#include <fstream>
#include "fonts.h"
#include "texture.h"

class Global {
public:
    int xres, yres;
};

class Ship {
public:
    float health;
};

class Game {
public:
    Ship ship;
    GLuint stardustTexture;
    GLuint ufoTexture;
    int showStardust;
};

extern Global gl;
extern Game g;

using namespace std;

//ufo variables
//GLuint ufoTexture;
//int ufoWidth, ufoHeight;

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

//void initTextures() 
//{
    //load texture here
//    ufoTexture = loadTexture("ufo.png", ufoWidth, ufoHeight);
//    glBindTexture(GL_TEXTURE_2D, ufoTexture);
//}


/*
GLuint loadTexture(const char *filename, int &width, int &height) 
{
    ifstream file(filename, ios::binary);
    if (!file) {
        cout << "Error: cannot open " << filename << endl;
        return 0;
    }

    // read ppm header
    string magic;
    // read format should be P6
    file >> magic;
    if (magic != "P6") {
        cout << "invalid PPM file" << endl;
        return 0;
    }

    file >> width >> height;
    int maxColor;
    file >> maxColor;
    file.ignore();  // skip single whitespace

    int dataSize = width * height * 3; // 3 bytes per pixel (RGB)
    unsigned char *data = new unsigned char[dataSize];

    // read image pixel data
    file.read(reinterpret_cast<char *>(data), dataSize);
    file.close();

    // generate OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
            GL_UNSIGNED_BYTE, data);

    delete[] data;
    return textureID;
}
*/

// --------------------------------------------------------------------
// added health bar function
// --------------------------------------------------------------------
void drawHealthBar(float health)
{
    if (health > 1.0f)
        health = 1.0f;
    if (health < 0.0f)
        health = 0.0f;

    // position for health bar at bottom right corner
    float healthBarWidth = 200.0f;
    float healthBarHeight = 15.0f;
    float barX = gl.xres - healthBarWidth;     // shows 10px from the right
    float barY = 10;    // shows 10px from the bottom

    Rect r;
    r.bot = barY + healthBarHeight + 5;
    r.left = barX;
    r.center = 0;
    ggprint8b(&r, 16, 0x00ffff00, "HEALTH");

    // draw background bar (r, g, b)
    glColor3f(0.0f, 0.0f, 0.0f);    // black
    glBegin(GL_QUADS);
    glVertex2f(barX, barY);
    glVertex2f(barX + healthBarWidth, barY);
    glVertex2f(barX + healthBarWidth, barY + healthBarHeight);
    glVertex2f(barX, barY + healthBarHeight);
    glEnd();

    // draw red health bar based on current ship health
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(barX, barY);
    glVertex2f(barX + (healthBarWidth * g.ship.health), barY);
    glVertex2f(barX + (healthBarWidth * g.ship.health), barY + healthBarHeight);
    glVertex2f(barX, barY + healthBarHeight);
    glEnd();

    // draw black lines to represent increments of health lost
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


