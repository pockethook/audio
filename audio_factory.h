#pragma once

#include "audio.h"
#include "alsa_audio.h"
#include "sdl_audio.h"
#include "sfml_audio.h"

#include <string>
#include <unordered_map>
#include <functional>

using make_backend =
	std::function<Audio*(const unsigned sample_rate, const std::string &format,
	                     const unsigned channels, const unsigned samples,
	                     RingBuffer* const ring)>;

const std::unordered_map<std::string, make_backend> backends{
	{"alsa", make_alsa},
	{"sdl", make_sdl},
	{"sfml", make_sfml},
};

Audio* make_audio(const std::string &backend,
                  const unsigned sample_rate, const std::string &format,
                  const unsigned channels, const unsigned samples,
                  RingBuffer* const ring) {
	return backends.at(backend)(sample_rate, format, channels, samples, ring);
}
