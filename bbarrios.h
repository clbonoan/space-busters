#ifndef BBARRIOS_H
#define BBARRIOS_H

#include <AL/al.h>
#include <AL/alc.h>

extern ALuint laserSource;
extern ALuint laserBuffer;
extern ALuint enemyDieSource;
extern ALuint enemyDieBuffer;

void playEnemyDieSound();
bool initOpenAL();
void shutdownOpenAL();
bool loadWavFile(const char* filename, ALuint &buffer);
void playLaserSound();
void playThemeMusic();
void stopThemeMusic();

#endif


