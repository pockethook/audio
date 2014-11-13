#pragma once

#include "audio.h"
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

class ALSAAudio : public Audio {
private:
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
	virtual ~ALSAAudio() override;
	virtual void operator()() override;

private:
	void run();
};

Audio* make_alsa(const unsigned sample_rate, const std::string &format,
                 const unsigned channels, const unsigned samples,
                 RingBuffer* const ring);
