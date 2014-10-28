#include "sfml_audio.h"


SFMLAudio::SFMLAudio(const unsigned sample_rate, const std::string &format,
		             const unsigned channels, const unsigned samples,
                     RingBuffer* const ring) :
		ring_{ring} {
	initialize(channels, sample_rate);
}

void SFMLAudio::operator()() {
	play();
}
