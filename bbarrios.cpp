#include "fonts.h"
#include "bbarrios.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

void show_bbarrios(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "Bryan - just a chill guy:");
}

ALCdevice *audioDevice = nullptr;
ALCcontext *audioContext = nullptr;
ALuint laserSource;
ALuint laserBuffer;

bool initOpenAL() {
    audioDevice = alcOpenDevice(nullptr);
    if (!audioDevice) {
        std::cerr << "OpenAL: Failed to open audio device\n";
        return false;
    }

    audioContext = alcCreateContext(audioDevice, nullptr);
    if (!audioContext || !alcMakeContextCurrent(audioContext)) {
        std::cerr << "OpenAL: Failed to create or set context\n";
        return false;
    }

    alGenSources(1, &laserSource);
    if (!loadWavFile("laser1.wav", laserBuffer)) {
        std::cerr << "Failed to load laser1.wav\n";
        return false;
    }

    std::cout << "OpenAL initialized successfully.\n";
    return true;
}

void shutdownOpenAL() {
    alDeleteSources(1, &laserSource);
    alDeleteBuffers(1, &laserBuffer);
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
            // Skip non-data chunk
            file.seekg(sectionSize, std::ios::cur);
        }
    }
}

void playLaserSound() {
    alSourcei(laserSource, AL_BUFFER, laserBuffer);
    alSourcePlay(laserSource);
}

