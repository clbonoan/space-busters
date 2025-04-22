#ifndef BBARRIOS_H
#define BBARRIOS_H

#include <AL/al.h>
#include <AL/alc.h>

extern ALuint laserSource;
extern ALuint laserBuffer;

bool initOpenAL();
void shutdownOpenAL();
bool loadWavFile(const char* filename, ALuint &buffer);
void playLaserSound();

#endif

