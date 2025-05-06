#include "fonts.h"
#include "bbarrios.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <GL/gl.h>
#include <GL/glx.h>
#include <cmath>
#include <ctime>

// Credits
void show_bbarrios(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Bryan - just a chill guy:");
}


// Crash Animation variables 
bool gameOverReady = false;
bool crash_animation_active = false;
float crash_center_x = 0.0f, crash_center_y = 0.0f;
float crash_timer = 0.0f;
const float CRASH_DURATION = 0.5f;
struct timespec crash_start_time;
//struct timespec lastExplosionTime = {0,0};
//const double explostionCooldown = 0.5



void resetCrashState()
{
    crash_animation_active = false;
    crash_timer = 0.0f;
}

void startCrashAnimation(float x, float y)
{
    crash_center_x = x;
    crash_center_y = y;
    crash_timer = CRASH_DURATION;
    crash_animation_active = true;
    clock_gettime(CLOCK_REALTIME, &crash_start_time);
}

void updateCrashAnimation()
{
    if (!crash_animation_active)
        return;

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double elapsed = (now.tv_sec - crash_start_time.tv_sec) +
                     (now.tv_nsec - crash_start_time.tv_nsec) / 1e9;

    if (elapsed >= CRASH_DURATION) {
        crash_animation_active = false;
        gameOverReady = true;
    }
}

bool isCrashDone()
{
    return !crash_animation_active;
}

// random float between 0 and 1
inline float rnd()
{
    return ((float)rand()) / (float)RAND_MAX;
}

// renders crash animation
void drawCrashAnimation()
{
    if (!crash_animation_active)
        return;

    glPushMatrix();
    glTranslatef(crash_center_x, crash_center_y, 0.0f);

    // Yellow spark burst using triangle fans (simple flare look)
    glColor3ub(255, 255, 0); // bright yellow
    for (int i = 0; i < 20; i++) {
        float ang = rnd() * 2.0f * M_PI;
        float r = 20.0f + rnd() * 30.0f;
        float x = cos(ang) * r;
        float y = sin(ang) * r;

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f); // center
        for (int j = 0; j <= 10; j++) {
            float theta = (float)j / 10.0f * 2.0f * M_PI;
            glVertex2f(x + cos(theta) * 2.0f, y + sin(theta) * 2.0f);
        }
        glEnd();
    }

    // Extra spark lines
    for (int i = 0; i < 50; ++i) {
        float ang = rnd() * 2.0f * M_PI;
        float x = cos(ang) * crash_timer * 30.0f;
        float y = sin(ang) * crash_timer * 30.0f;

        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(rnd() * 12.0f - 6.0f, rnd() * 12.0f - 6.0f);
        glVertex2f(x + rnd() * 10.0f - 5.0f, y + rnd() * 10.0f - 5.0f);
        glEnd();
    }

    glPopMatrix();
}


//-- OpenAL Stuff

ALCdevice *audioDevice = nullptr;
ALCcontext *audioContext = nullptr;

ALuint laserSource, laserBuffer;
ALuint enemyDieSource, enemyDieBuffer;
ALuint musicSource, musicBuffer;

//--initializes OpenAL and loads sounds
bool initOpenAL()
{
    audioDevice = alcOpenDevice(nullptr);
    if (!audioDevice) {
        std::cerr << "couldn't open audio device\n";
        return false;
    }

    audioContext = alcCreateContext(audioDevice, nullptr);
    if (!audioContext || !alcMakeContextCurrent(audioContext)) {
        std::cerr << "OpenAL failed, sorry\n";
        return false;
    }

    alGenSources(1, &laserSource);
    if (!loadWavFile("audio/laser1.wav", laserBuffer)) {
        std::cerr << "couldn't load laser sound\n";
        return false;
    }
    alSourcei(laserSource, AL_BUFFER, laserBuffer);

    alGenSources(1, &enemyDieSource);
    if (!loadWavFile("audio/enemy_explode2.wav", enemyDieBuffer)) {
        std::cerr << "couldn't load enemy explode sound\n";
        return false;
    }
    alSourcei(enemyDieSource, AL_BUFFER, enemyDieBuffer);

    alGenSources(1, &musicSource);
    if (!loadWavFile("audio/menu_theme.wav", musicBuffer)) {
        std::cerr << "couldn't load menu theme music\n";
        return false;
    }
    alSourcei(musicSource, AL_BUFFER, musicBuffer);

    std::cout << "OpenAL initialized.\n";
    return true;
}




// Releases OpenAL resources
void shutdownOpenAL()
{
    alDeleteSources(1, &laserSource);
    alDeleteBuffers(1, &laserBuffer);
    alDeleteSources(1, &enemyDieSource);
    alDeleteBuffers(1, &enemyDieBuffer);
    alDeleteSources(1, &musicSource);
    alDeleteBuffers(1, &musicBuffer);

    alcMakeContextCurrent(nullptr);
    if (audioContext)
        alcDestroyContext(audioContext);
    if (audioDevice)
        alcCloseDevice(audioDevice);

    std::cout << "OpenAL shut down.\n";
}

// loads wav file into buffer
bool loadWavFile(const char *filename, ALuint &buffer)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "ERROR: Cannot open file " << filename << "\n";
        return false;
    }

    char chunk[4];
    file.read(chunk, 4);
    if (strncmp(chunk, "RIFF", 4) != 0) {
        std::cerr << "ERROR: Not a RIFF file\n";
        return false;
    }

    file.ignore(4);
    file.read(chunk, 4);
    if (strncmp(chunk, "WAVE", 4) != 0) {
        std::cerr << "ERROR: Not a WAVE file\n";
        return false;
    }

    file.read(chunk, 4);
    int fmtSize;
    file.read((char *)&fmtSize, 4);

    short formatType, channels, blockAlign, bitsPerSample;
    int sampleRate, byteRate;

    file.read((char *)&formatType, 2);
    file.read((char *)&channels, 2);
    file.read((char *)&sampleRate, 4);
    file.read((char *)&byteRate, 4);
    file.read((char *)&blockAlign, 2);
    file.read((char *)&bitsPerSample, 2);

    if (fmtSize > 16)
        file.ignore(fmtSize - 16);

    while (true) {
        file.read(chunk, 4);
        if (file.eof()) {
            std::cerr << "ERROR: EOF before data chunk\n";
            return false;
        }

        int sectionSize;
        file.read((char *)&sectionSize, 4);

        if (strncmp(chunk, "data", 4) == 0) {
            std::vector<char> data(sectionSize);
            file.read(data.data(), sectionSize);

            ALenum format = (channels == 1)
                ? (bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16)
                : (bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);

            alGenBuffers(1, &buffer);
            alBufferData(buffer, format, data.data(), sectionSize, sampleRate);

            std::cout << "WAV loaded: " << filename << "\n";
            return true;
        } else {
            file.seekg(sectionSize, std::ios::cur);
        }
    }
}


// --sound wrappers
void playLaserSound()
{
    alSourcei(laserSource, AL_BUFFER, laserBuffer);
    alSourcePlay(laserSource);
}


void playEnemyDieSound() {
    ALint state;
    alGetSourcei(enemyDieSource, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING) {
        alSourcePlay(enemyDieSource);
    }
}

/*void playEnemyDieSound()
{
    alSourcei(enemyDieSource, AL_BUFFER, enemyDieBuffer);
    alSourcePlay(enemyDieSource);
}*/

void playThemeMusic()
{
    alSourcei(musicSource, AL_LOOPING, AL_TRUE);
    alSourcePlay(musicSource);
}

void stopThemeMusic()
{
    alSourceStop(musicSource);
}

