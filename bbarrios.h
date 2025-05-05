#ifndef BBARRIOS_H
#define BBARRIOS_H

#include <AL/al.h>
#include <AL/alc.h>

extern ALuint laserSource;
extern ALuint laserBuffer;
extern ALuint enemyDieSource;
extern ALuint enemyDieBuffer;
extern bool crash_animation_active;
extern bool gameOverReady;

void startCrashAnimation(float x, float y);
void drawCrashAnimation();
void updateCrashAnimation();
bool isCrashDone();
void resetCrashState();

void playEnemyDieSound();
bool initOpenAL();
void shutdownOpenAL();
bool loadWavFile(const char* filename, ALuint &buffer);
void playLaserSound();
void playThemeMusic();
void stopThemeMusic();

#endif


