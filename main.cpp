#include "ring_buffer.h"
#include "sdl_audio.h"

#include <SDL2/SDL.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::unique_ptr;

void read_file(const string &file_name, RingBuffer* ring) {
	ifstream input{file_name, std::ifstream::binary};
	const size_t size {8192};
	uint8_t temp[size];
	input.seekg(48 * 4);
	while (input) {
		input.read(reinterpret_cast<char*>(temp), size);
		ring->push(temp, size);
	}
}

int main(const int argc, char** argv) {

	if (argc < 2) {
		cerr << "Usage: player file_name.wav" << endl;
		return -1;
	}

	const string file_name{argv[1]};
	const size_t ring_buffer_size{1048576};
	const unsigned frequency{48000}; 

	unique_ptr<RingBuffer> ring{new RingBufferLock{ring_buffer_size}};
	
	SDLAudio audio{frequency, ring.get()};
	audio();
	read_file(file_name, ring.get());

	return 0;
}
