#pragma once

#include "ring_buffer.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

const std::unordered_map<std::string, SDL_AudioFormat> formats {
	{"u8", AUDIO_U8},
	{"s16", AUDIO_S16},
	{"s32", AUDIO_S32},
	{"flt", AUDIO_F32},
};

// Wrapper around SDL audio device handlers
class SDLAudioDevice {
private:
	SDL_AudioSpec spec_;
	SDL_AudioDeviceID device_ {0};

public:
	SDLAudioDevice(SDL_AudioSpec* spec);
	~SDLAudioDevice();
	void operator()();
	SDL_AudioSpec spec() const;
};

std::ostream &operator<<(std::ostream &o, const SDLAudioDevice &device);

class SDLAudio {
private:
	RingBuffer* ring_;
	SDL_AudioSpec spec_;
	std::unique_ptr<SDLAudioDevice> audio_;

public:
	SDLAudio(const unsigned sample_rate, const std::string &format,
	         const unsigned channels, const unsigned samples,
	         RingBuffer* const ring); 
	~SDLAudio();
	void operator()();
	SDLAudioDevice audio() const;

private:
	static void audio_callback(void *userdata, Uint8 *stream, int len);
};

std::ostream &operator<<(std::ostream &o, const SDLAudio &audio);
