#include "alsa_audio.h"

#include <alsa/asoundlib.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <thread>

using std::runtime_error;
using std::min;
using std::thread;
using std::fill;

void error(const int ret) {
	if (ret < 0) {
		throw runtime_error(snd_strerror(ret));
	}
}

ALSAAudio::ALSAAudio(const unsigned sample_rate, const std::string &format,
	                 const unsigned channels, const unsigned samples,
	                 RingBuffer* const ring) :
		ring_{ring},
		format_{format},
		channels_{channels},
		samples_{samples} {
	int ret;
	if ((ret = snd_pcm_open(
			&handle_, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_hw_params_malloc(&hw_params_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_hw_params_any(handle_, hw_params_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}

	// XXX: Hook up SND_PCM_ACCESS_RW_NOINTERLEAVED
	// XXX: Consider mmap
	if ((ret = snd_pcm_hw_params_set_access(
			handle_, hw_params_, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_hw_params_set_format(
			handle_, hw_params_, alsa_formats.at(format_))) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	unsigned rate = sample_rate;
	int val = 0;
	if ((ret = snd_pcm_hw_params_set_rate(
			handle_, hw_params_, rate, val)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_hw_params_set_channels(
			handle_, hw_params_, channels)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_hw_params(handle_, hw_params_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	snd_pcm_hw_params_free(hw_params_);


	if ((ret = snd_pcm_sw_params_malloc(&sw_params_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_sw_params_current(handle_, sw_params_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_sw_params_set_avail_min(
			handle_, sw_params_, samples_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_sw_params_set_start_threshold(
			handle_, sw_params_, 0)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
	if ((ret = snd_pcm_sw_params(handle_, sw_params_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}


	if ((ret = snd_pcm_prepare(handle_)) < 0) {
		throw runtime_error(snd_strerror(ret));
	}
}

ALSAAudio::~ALSAAudio() {
	run_.join();
	snd_pcm_close(handle_);
}

void ALSAAudio::operator()() {
	run_ = thread(&ALSAAudio::run, this);
}

void ALSAAudio::run() {
	for (unsigned i = 0;; ++i) {
		int ret;
		if ((ret = snd_pcm_wait(handle_, 1000)) < 0) {
			throw runtime_error(snd_strerror(ret));
		}	           

		// Find out how much space is available for playback data 
		snd_pcm_sframes_t frames_to_deliver;
		if ((frames_to_deliver = snd_pcm_avail_update(handle_)) < 0) {
			if (frames_to_deliver == -EPIPE) {
				throw runtime_error("Error in xrun.");
			} else {
				throw runtime_error("Error in ALSA avail.");
			}
		}

		frames_to_deliver = min(long{frames_to_deliver}, long{samples_});

		// Deliver the data
		unsigned size =
		   	channels_ * format_sizes.at(format_) * frames_to_deliver;
		std::unique_ptr<uint8_t> buffer(new uint8_t[size]{});
		ring_->pop(buffer.get(), size);
		if ((ret = snd_pcm_writei(
				handle_, buffer.get(), frames_to_deliver)) < 0) {
			throw runtime_error(snd_strerror(ret));
		}
	}
}
