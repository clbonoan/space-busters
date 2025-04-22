#include "fonts.h"
//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <cstring>



void show_bbarrios(Rect *r)
{
    ggprint8b(r, 16, 0x00ff00ff, "bryan - just a chill guy:");
}


/*
ALCdevice *audioDevice = nullptr;
ALCcontext *audioContext = nullptr;
ALuint audioSource;

bool initOpenAL() {
    audioDevice = alcOpenDevice(nullptr);
    if (!audioDevice) {
        std::cerr << "Failed to open OpenAL device\n";
        return false;
    }
    audioContext = alcCreateContext(audioDevice, nullptr);
    if (!audioContext || !alcMakeContextCurrent(audioContext)) {
        std::cerr << "Failed to create OpenAL context\n";
        return false;
    }
    alGenSources(1, &audioSource);
    return true;
}

void shutdownOpenAL() {
    alDeleteSources(1, &audioSource);
    if (audioContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(audioContext);
    }
    if (audioDevice)
        alcCloseDevice(audioDevice);
}

bool loadWavFile(const std::string& filename, ALuint &buffer) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    char chunkId[4];
    file.read(chunkId, 4);
    if (std::strncmp(chunkId, "RIFF", 4)) return false;

    file.ignore(4); // skip chunk size

    char format[4];
    file.read(format, 4);
    if (std::strncmp(format, "WAVE", 4)) return false;

    char subchunk1Id[4];
    file.read(subchunk1Id, 4);
    file.ignore(4); // subchunk1 size

    short audioFormat, numChannels, bitsPerSample;
    int sampleRate, byteRate;
    short blockAlign;

    file.read((char*)&audioFormat, 2);
    file.read((char*)&numChannels, 2);
    file.read((char*)&sampleRate, 4);
    file.read((char*)&byteRate, 4);
    file.read((char*)&blockAlign, 2);
    file.read((char*)&bitsPerSample, 2);

    file.ignore(8); // skip "data" + data size

    std::vector<char> data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    ALenum formatEnum;
    if (numChannels == 1)
        formatEnum = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    else
        formatEnum = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

    alGenBuffers(1, &buffer);
    alBufferData(buffer, formatEnum, data.data(), data.size(), sampleRate);
    return true;
}

void playSound(ALuint buffer) {
    alSourcei(audioSource, AL_BUFFER, buffer);
    alSourcePlay(audioSource);
}
*/
