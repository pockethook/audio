#pragma once

#include "ring_buffer.h"

#include <SFML/Audio.hpp>

//#include <iomanip>
//#include <iostream>
//#include <vector>
//#include <cmath>
//
//
//int main() {
//    sf::SoundBuffer buffer;
//    //if (!buffer.loadFromFile("output.wav"))
//    //    return 1;
//	std::vector<sf::Int16> samples;
//	for (unsigned i = 0; i < 123456789; ++i) {
//		samples.push_back(2000 * rand());
//	}
//	std::cout << samples.size() << std::endl;
//	buffer.loadFromSamples(&samples[0], samples.size(), 2, 48000);
//    sf::Sound sound(buffer);
//    sound.play();
//    sf::sleep(sf::seconds(buffer.getDuration().asSeconds()));
//}
//
//
//#pragma once
//
//
//#include <SDL2/SDL.h>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

//const std::unordered_map<std::string, int> formats {
//	{"s16", 0},
//};

class SFMLAudio : public sf::SoundStream {
private:
	RingBuffer* ring_;
	int16_t buffer[1048576];

public:
	SFMLAudio(const unsigned sample_rate, const std::string &format,
	          const unsigned channels, const unsigned samples,
	          RingBuffer* const ring); 
	void operator()();

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
