#include "sdl_audio.h"
#include "ring_buffer.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

SDLAudioDevice::SDLAudioDevice(SDL_AudioSpec* spec) :
		spec_(*spec),
		device_{SDL_OpenAudioDevice(
			nullptr, 0, spec, nullptr, 0)} {
	if (!device_) {
		throw std::runtime_error(SDL_GetError());
	}
}

SDLAudioDevice::~SDLAudioDevice() {
	SDL_CloseAudioDevice(device_);
}

void SDLAudioDevice::operator()() {
	SDL_PauseAudioDevice(device_, 0);
}

SDL_AudioSpec SDLAudioDevice::spec() const {
	return spec_;
}

std::ostream &operator<<(std::ostream &o, const SDLAudioDevice &device) {
	auto spec = device.spec();
	o << "sample_rate " << spec.freq << std::endl;
	o << "format " << (spec.format == AUDIO_F32LSB) << std::endl;
	o << "channels " << static_cast<int>(spec.channels) << std::endl;
	o << "samples " << spec.samples << std::endl;
	return o;
}

SDLAudio::SDLAudio(const unsigned sample_rate, const std::string &format,
		 const unsigned channels, const unsigned samples,
		 RingBuffer* const ring) :
		Audio{ring},
		spec_{static_cast<int>(sample_rate), formats.at(format),
			  static_cast<Uint8>(channels), 0,
			  static_cast<Uint16>(samples),
			  0, 0, &audio_callback, ring_} {
	if (SDL_Init(SDL_INIT_AUDIO)) {
		throw std::runtime_error(SDL_GetError());
	}
	audio_.reset(new SDLAudioDevice{&spec_});
}

SDLAudio::~SDLAudio() {
	SDL_Quit();
}

void SDLAudio::operator()() {
	(*audio_)();
}

SDLAudioDevice SDLAudio::audio() const {
	return *audio_;
}

void SDLAudio::audio_callback(void *userdata, Uint8 *stream, int len) {
	std::fill(&stream[0], &stream[len], 0);
	RingBuffer *ring = reinterpret_cast<RingBuffer*>(userdata);
	ring->pop(stream, len);
}

std::ostream &operator<<(std::ostream &o, const SDLAudio &audio) {
	o << audio.audio() << std::endl;
	return o;
}

Audio* make_sdl(const unsigned sample_rate, const std::string &format,
                 const unsigned channels, const unsigned samples,
                 RingBuffer* const ring) {
	return new SDLAudio{sample_rate, format, channels, samples, ring};
}
