#include "audio_manager.h"
#include <iostream>
#include "data.h"

/*/// forward declarations ///*/
char* loadWAV(std::string filename, int32_t& channels, int32_t& sampleRate, int32_t& bitsPerChannel, int32_t& size);

/////////////////////////////////////
/// class: AudioManager /////////////
/////////////////////////////////////

ALCdevice* AudioManager::mDevice;
ALCcontext* AudioManager::mContext;
uint32_t* AudioManager::buf;
uint32_t* AudioManager::source;

void AudioManager::init()
{
	mDevice = alcOpenDevice(nullptr);

	ASSERT(mDevice, "[OPENAL]: unable to find audio device!");
	mContext = alcCreateContext(mDevice, nullptr);
	alcMakeContextCurrent(mContext);

	ASSERT(alIsExtensionPresent("EAX2.0"), "[OPENAL]: audio device doesn't support EAX 2.0!");

    buf = new uint32_t[5];
    source = new uint32_t[5];
	alGenBuffers(5, buf);
	alGenSources(5, source);
	ASSERT(alGetError() == AL_NO_ERROR, "[OPENAL]: unable to create buffers!");

    /* create 5 openal audio sources */
    for (size_t i = 0; i < 5; i++)
    {
	    alBufferData(buf[i], AL_FORMAT_MONO8, (const void*)sAudios[i].data.data(), sAudios[i].size, sAudios[i].frequency);

        alSourcef(source[i], AL_PITCH, 1);
        alSourcef(source[i], AL_GAIN, 1);
        alSource3f(source[i], AL_POSITION, 0, 0, 0);
        alSource3f(source[i], AL_VELOCITY, 0, 0, 0);
        alSourcei(source[i], AL_LOOPING, AL_FALSE);
        alSourcei(source[i], AL_BUFFER, buf[i]);
    }

    alSourcef(source[PACMAN_EAT_FRUIT], AL_PITCH, 1.0f);
    alSourcef(source[PACMAN_EAT_FRUIT], AL_GAIN, 0.3f);

    /* set listener attributes */
	ALfloat listenerPos[] = { 0.0,0.0,0.0 };
	ALfloat listenerVel[] = { 0.0,0.0,0.0 };
	ALfloat listenerOri[] = { 0.0,0.0,-1.0, 0.0,1.0,0.0 };
    alListenerf(AL_GAIN, 1);
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
}

void AudioManager::shutDown()
{
	alDeleteSources(5, source);
	alDeleteBuffers(5, buf);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);

    delete[] buf;
    delete[] source;
}

void AudioManager::playSound(eSound sound)
{
    alSourcePlay(source[(int32_t)sound]);
}

void AudioManager::waitForEnd(eSound sound)
{
    int32_t state = 0;
    alGetSourcei(source[(int32_t)sound], AL_SOURCE_STATE, &state);
    while(state == AL_PLAYING)
        alGetSourcei(source[(int32_t)sound], AL_SOURCE_STATE, &state);
}

/*/// helper functions ///*/
bool isBigEndian()
{
    int a = 1;
    return !((char*)&a)[0];
}

int32_t convertToInt(char* buffer, int32_t len, bool bigEndian)
{
    int32_t a = 0;

    if (!bigEndian)
        for (int i = 0; i < len; ++i)
            ((char*)&a)[i] = buffer[i];
    else
        for (int i = 0; i < len; ++i)
            ((char*)&a)[3 - i] = buffer[i];

    return a;
}

char* loadWAV(std::string filename, int32_t& channels, int32_t& sampleRate, int32_t& bitsPerChannel, int32_t& size)
{
    char buffer[4];

    bool endianness = isBigEndian();

    std::ifstream in(filename.c_str(), std::ios::binary);
    in.read(buffer, 4);

    ERR(!in.fail(), "failed to open audio file!");

    ASSERT((strncmp(buffer, "RIFF", 4) == 0), ".WAV file metadata different! doesn't contain \"RIFF\"!");

    in.read(buffer, 4);//size of file. Not used. Read it to skip over it.  

    in.read(buffer, 4);//Format, should be WAVE
    ASSERT((strncmp(buffer, "WAVE", 4) == 0), ".WAV file metadata different! doesn't contain \"WAVE\"!");

    in.read(buffer, 4);//Format Space Marker. should equal fmt (space)
    ASSERT((strncmp(buffer, "fmt ", 4) == 0), ".WAV file metadata different! doesn't contain \"fmt \"!");

    in.read(buffer, 4);//Length of format data. Should be 16 for PCM, meaning uncompressed.
    ASSERT((convertToInt(buffer, 4, endianness) == 16), ".WAV file metadata different! format not 16!");

    in.read(buffer, 2);//Type of format, 1 = PCM
    ASSERT((convertToInt(buffer, 2, endianness) == 1), ".WAV file metadata different! format (PCM) not 1!");

    in.read(buffer, 2);//Get number of channels. 
    channels = convertToInt(buffer, 2, endianness);

    in.read(buffer, 4);//Get sampler rate. 
    sampleRate = convertToInt(buffer, 4, endianness);

    //Skip Byte Rate and Block Align. Maybe use later?
    in.read(buffer, 4);//Block Align
    in.read(buffer, 2);//ByteRate

    in.read(buffer, 2);//Get Bits Per Sample
    bitsPerChannel = convertToInt(buffer, 2, endianness);

    //Skip character data, which marks the start of the data that we care about. 
    in.read(buffer, 4);//"data" chunk. 

    in.read(buffer, 4); //Get size of the data
    size = convertToInt(buffer, 4, endianness);

    ASSERT((size > 0), "audio chunk size less than 0!");

    char* data = new char[size];
    in.read(data, size);//Read audio data into buffer, return.

    in.close();

    return data;
}
