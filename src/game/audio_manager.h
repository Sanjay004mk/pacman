#pragma once
#include <openAL\alc.h>
#include <openAL\al.h>

#include "utils.h"

class AudioManager
{
public:
	AudioManager();
	~AudioManager();

	static void playSound(eSound sound);
	static void waitForEnd(eSound sound);
	static void init();
	static void shutDown();
private:
	static ALCdevice* mDevice;
	static ALCcontext* mContext;
	static uint32_t* buf;
	static uint32_t* source;
};