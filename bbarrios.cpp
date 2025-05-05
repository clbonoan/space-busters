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

void show_bbarrios(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Bryan - just a chill guy:");
}

ALCdevice *audioDevice = nullptr;
ALCcontext *audioContext = nullptr;

ALuint laserSource;
ALuint laserBuffer;

ALuint enemyDieSource;
ALuint enemyDieBuffer;

ALuint musicSource;
ALuint musicBuffer;

inline float rnd() {
    return ((float)rand()) / (float)RAND_MAX;
}


bool gameOverReady = false;
bool crash_animation_active = false;
float crash_center_x = 0.0f, crash_center_y = 0.0f;
float crash_timer = 0.0f;
const float CRASH_DURATION= 1.2f;
struct timespec crash_start_time;


void startCrashAnimation(float x, float y) {
    crash_center_x = x;
    crash_center_y = y;
    crash_timer = CRASH_DURATION;
    crash_animation_active = true;
    clock_gettime(CLOCK_REALTIME, &crash_start_time);
}

void updateCrashAnimation() {
    if (!crash_animation_active) return;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double elapsed = (now.tv_sec - crash_start_time.tv_sec) +
                     (now.tv_nsec - crash_start_time.tv_nsec) / 1e9;
    if (elapsed >= CRASH_DURATION) {
        crash_animation_active = false;
        gameOverReady = true;
    }
}

bool isCrashDone() {
    return !crash_animation_active;
}

void drawCrashAnimation() {
    if (!crash_animation_active) return;

    glPushMatrix();
    glTranslatef(crash_center_x, crash_center_y, 0.0f);

    for (int i = 0; i < 30; i++) {
        float a = ((float)rand() / RAND_MAX) * 2 * M_PI;
        float r = 30.0f * ((float)rand() / RAND_MAX);
        float x = cos(a) * r;
        float y = sin(a) * r;
        glColor3ub(255, 255, 0);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x, y);
            for (int j = 0; j <= 10; j++) {
                float theta = j / 10.0f * 2 * M_PI;
                glVertex2f(x + cos(theta) * 2, y + sin(theta) * 2);
            }
        glEnd();
    }

    glPopMatrix();
}


//opens OpenAL
bool initOpenAL() {
    audioDevice = alcOpenDevice(nullptr);
    if (!audioDevice) {
        std::cerr << "couldnt open audio device\n";
        return false;
    }

    audioContext = alcCreateContext(audioDevice, nullptr);
    if (!audioContext || !alcMakeContextCurrent(audioContext)) {
        std::cerr << "OpenAL failed, sorry\n";
        return false;
    }

    // loads laser sound
    alGenSources(1, &laserSource);
    if (!loadWavFile("audio/laser1.wav", laserBuffer)) {
        std::cerr << "couldnt load laser sound\n";
        return false;
    }
    alSourcei(laserSource, AL_BUFFER, laserBuffer);

    // loads enemy explodes sound
    alGenSources(1, &enemyDieSource);
    if (!loadWavFile("audio/enemy_explode.wav", enemyDieBuffer)) {
        std::cerr << "couldnt load enemy explode sound\n";
        return false;
    }
    alSourcei(enemyDieSource, AL_BUFFER, enemyDieBuffer);

    // loads menu music
    alGenSources(1, &musicSource);
    if (!loadWavFile("audio/pudding.wav", musicBuffer)) {
        std::cerr << "couldnt load menu theme music\n";
        return false;
    }
    alSourcei(musicSource, AL_BUFFER, musicBuffer);


    std::cout << "OpenAl worked fine.\n";
    return true;

}



void shutdownOpenAL() {
    alDeleteSources(1, &enemyDieSource);
    alDeleteBuffers(1, &enemyDieBuffer);
    alDeleteSources(1, &laserSource);
    alDeleteBuffers(1, &laserBuffer);
    alDeleteSources(1, &musicSource);
    alDeleteBuffers(1, &musicBuffer);
    alDeleteSources(1, &musicSource);  
    alDeleteBuffers(1, &musicBuffer);    


    alcMakeContextCurrent(nullptr);
    if (audioContext) alcDestroyContext(audioContext);
    if (audioDevice) alcCloseDevice(audioDevice);

    std::cout << "OpenAL shut down.\n";
    }

bool loadWavFile(const char* filename, ALuint &buffer) {
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
    file.read((char*)&fmtSize, 4);

    short formatType, channels, blockAlign, bitsPerSample;
    int sampleRate, byteRate;

    file.read((char*)&formatType, 2);
    file.read((char*)&channels, 2);
    file.read((char*)&sampleRate, 4);
    file.read((char*)&byteRate, 4);
    file.read((char*)&blockAlign, 2);
    file.read((char*)&bitsPerSample, 2);

    if (fmtSize > 16) file.ignore(fmtSize - 16);


    while (true) {
        file.read(chunk, 4);
        if (file.eof()) {
            std::cerr << "ERROR: EOF before data chunk\n";
            return false;
        }

        int sectionSize;
        file.read((char*)&sectionSize, 4);

        if (strncmp(chunk, "data", 4) == 0) {
            std::vector<char> data(sectionSize);
            file.read(data.data(), sectionSize);

            ALenum format = (channels == 1)
                ? (bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16)
                : (bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);

            alGenBuffers(1, &buffer);
            alBufferData(buffer, format, data.data(), sectionSize, sampleRate);

            std::cout << "WAV loaded successfully: " << filename << "\n";
            return true;
        } else {
            file.seekg(sectionSize, std::ios::cur);
        }
    }
}

// play laser sound
void playLaserSound() {
    alSourcei(laserSource, AL_BUFFER, laserBuffer);
    alSourcePlay(laserSource);
}

// play explosion sound
void playEnemyDieSound() {
    alSourcei(enemyDieSource, AL_BUFFER, enemyDieBuffer);
    alSourcePlay(enemyDieSource);
}

// play theme music
void playThemeMusic() {
    alSourcei(musicSource, AL_LOOPING, AL_TRUE);
    alSourcePlay(musicSource);
}

// stop theme music
void stopThemeMusic() {
    alSourceStop(musicSource);
}
