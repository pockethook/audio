#pragma once

#include "audio.h"
#include "ring_buffer.h"

#include <SFML/Audio.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

class SFMLAudio : public sf::SoundStream , public Audio {
private:
	int16_t buffer[1048576];

public:
	SFMLAudio(const unsigned sample_rate, const std::string &format,
	          const unsigned channels, const unsigned samples,
	          RingBuffer* const ring); 
	~SFMLAudio() override;
	virtual void operator()() override;

private:
	virtual bool onGetData(Chunk &data) override {
		const size_t size {8192};
		data.sampleCount = size;
		ring_->pop(reinterpret_cast<uint8_t*>(&buffer[0]), 2 * size);
		data.samples = &buffer[0];
		return true;
	}
	virtual void onSeek(sf::Time timeOffset) {
		const size_t seek =
		   	timeOffset.asSeconds() * getSampleRate() * getChannelCount();
		ring_->pop(reinterpret_cast<uint8_t*>(&buffer[0]), seek / 2);
	}
};

Audio* make_sfml(const unsigned sample_rate, const std::string &format,
                 const unsigned channels, const unsigned samples,
                 RingBuffer* const ring);
