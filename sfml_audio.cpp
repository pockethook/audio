#include "sfml_audio.h"

SFMLAudio::SFMLAudio(const unsigned sample_rate, const std::string &format,
		             const unsigned channels, const unsigned samples,
                     RingBuffer* const ring) :
		Audio{ring} {
	initialize(channels, sample_rate);
}

SFMLAudio::~SFMLAudio() {
}

void SFMLAudio::operator()() {
	play();
}

Audio* make_sfml(const unsigned sample_rate, const std::string &format,
                 const unsigned channels, const unsigned samples,
                 RingBuffer* const ring) {
	return new SFMLAudio{sample_rate, format, channels, samples, ring};
}
