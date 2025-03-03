#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/gl.h>

extern GLuint ufoTexture;
extern int ufoWidth, ufoHeight;

//function prototypes
GLuint loadPPMTexture(const char *filename, int &width, int &height);
void initTextures();

#endif

