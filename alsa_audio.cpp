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
		Audio{ring},
		format_{format},
		channels_{channels},
		samples_{samples} {

	error(snd_pcm_open(&handle_, "default", SND_PCM_STREAM_PLAYBACK, 0));
	error(snd_pcm_hw_params_malloc(&hw_params_));
	error(snd_pcm_hw_params_any(handle_, hw_params_));

	// XXX: Hook up SND_PCM_ACCESS_RW_NOINTERLEAVED
	// XXX: Consider mmap
	error(snd_pcm_hw_params_set_access(
		handle_, hw_params_, SND_PCM_ACCESS_RW_INTERLEAVED));
	error(snd_pcm_hw_params_set_format(
		handle_, hw_params_, alsa_formats.at(format_))); 
	unsigned rate = sample_rate;
	int val = 0;
	error(snd_pcm_hw_params_set_rate(handle_, hw_params_, rate, val));
	error(snd_pcm_hw_params_set_channels(handle_, hw_params_, channels));
	error(snd_pcm_hw_params(handle_, hw_params_));
	snd_pcm_hw_params_free(hw_params_);

	error(snd_pcm_sw_params_malloc(&sw_params_));
	error(snd_pcm_sw_params_current(handle_, sw_params_));
	error(snd_pcm_sw_params_set_avail_min(handle_, sw_params_, samples_));
	error(snd_pcm_sw_params_set_start_threshold(handle_, sw_params_, 0));
	error(snd_pcm_sw_params(handle_, sw_params_));

	error(snd_pcm_prepare(handle_));
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
		error(snd_pcm_wait(handle_, 1000));

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
		if (!ring_->pop(buffer.get(), size)) {
			ring_->pop(buffer.get(), ring_->space_used() - 1);
			error(snd_pcm_writei(handle_, buffer.get(), frames_to_deliver));
			error(snd_pcm_wait(handle_, 1000));
			std::this_thread::sleep_for(std::chrono::seconds(1));
			break;
		}
		error(snd_pcm_writei(handle_, buffer.get(), frames_to_deliver));
	}
}

Audio* make_alsa(const unsigned sample_rate, const std::string &format,
                 const unsigned channels, const unsigned samples,
                 RingBuffer* const ring) {
	return new ALSAAudio{sample_rate, format, channels, samples, ring};
}
