#pragma once

#include "ring_buffer.h"

#include <SDL2/SDL.h>

#include <memory>
#include <stdexcept>

using std::runtime_error;
using std::unique_ptr;

// Wrapper around SDL audio device handlers
class SDLAudioDevice {
private:
	SDL_AudioDeviceID device_ {0};

public:
	SDLAudioDevice(SDL_AudioSpec* spec) :
			device_{SDL_OpenAudioDevice(
				nullptr, 0, spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE)} {
		if (!device_) {
			throw runtime_error("Could not open SDL audio device.");
		}
	}
	~SDLAudioDevice() {
		SDL_CloseAudioDevice(device_);
	}
	void operator()() {
		SDL_PauseAudioDevice(device_, 0);
	}
};

class SDLAudio {
private:
	RingBuffer* ring_;
	SDL_AudioSpec spec_;
	unique_ptr<SDLAudioDevice> audio_;

public:
	SDLAudio(const unsigned frequency, RingBuffer* const ring) :
			ring_{ring},
			spec_{static_cast<int>(frequency), AUDIO_S16, 2, 0, 4096,
				  0, 0, &audio_callback, ring_} {
		if (SDL_Init(SDL_INIT_AUDIO)) {
			SDL_Quit();		
			throw runtime_error("Could not initialize SDL.");
		}
		audio_.reset(new SDLAudioDevice{&spec_});
	}
	void operator()() {
		(*audio_)();
	}

private:
	static void audio_callback(void *userdata, Uint8 *stream, int len) {
		RingBuffer *ring = reinterpret_cast<RingBuffer*>(userdata);
		ring->pop(stream, len);
	}
};
