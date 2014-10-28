#pragma once

#include "ring_buffer.h"

#include <alsa/asoundlib.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include <thread>

const std::unordered_map<std::string, snd_pcm_format_t> alsa_formats {
	{"u8", SND_PCM_FORMAT_U8},
	{"s16", SND_PCM_FORMAT_S16},
	{"s32", SND_PCM_FORMAT_S32},
	{"flt", SND_PCM_FORMAT_FLOAT},
	{"dbl", SND_PCM_FORMAT_FLOAT64},
};

const std::unordered_map<std::string, unsigned> format_sizes {
	{"u8", sizeof(uint8_t)},
	{"s16", sizeof(int16_t)},
	{"s32", sizeof(int32_t)},
	{"flt", sizeof(float)},
	{"dbl", sizeof(double)},
};

class ALSAAudio {
private:
	RingBuffer* ring_;
	snd_pcm_t *handle_;
	snd_pcm_hw_params_t* hw_params_;
	snd_pcm_sw_params_t* sw_params_;
	std::thread run_;

	std::string format_;
	unsigned channels_;
	unsigned samples_;

public:
	ALSAAudio(const unsigned sample_rate, const std::string &format,
	         const unsigned channels, const unsigned samples,
	         RingBuffer* const ring); 
	~ALSAAudio();
	void operator()();

private:
	void run();
};
