#include "ring_buffer.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <thread>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::min;
using std::runtime_error;
using std::ifstream;
using std::unique_ptr;
using std::copy;

class OutputAudio {
private:
	SDL_AudioSpec spec_;
	SDL_AudioDeviceID device_ {0};

public:
	OutputAudio(SDL_AudioSpec spec) :
			spec_(spec),
			device_{SDL_OpenAudioDevice(
				nullptr, 0, &spec_, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE)} {
		if (!device_) {
			throw runtime_error("Could not open SDL audio device.");
		}
	}
};

Sint64 size(SDL_RWops* context) {
	cout << "size" << endl;
	return -1;
}

Sint64 seek(SDL_RWops* context, Sint64 offset, int whence) {
	cout << "seek " << offset << ' ' << whence << endl;
	return -1;
}

size_t read(SDL_RWops* context, void* buffer, size_t size, size_t length) {
	cout << "read " << size << ' ' << length << endl;
	auto ring = reinterpret_cast<RingBuffer*>(context->hidden.unknown.data1);
	ring->pop_lock(reinterpret_cast<uint8_t*>(buffer), size * length);
	return size;
}

int close(SDL_RWops* context) {
	cout << "close" << endl;
	SDL_FreeRW(context);
	return 0;
}

void audio_callback(void *userdata, Uint8 *stream, int len);

struct Buffer {
	Uint8 *position;
	Uint32 length;
};

int main(const int argc, char** argv) {


	const string usage {"player file_name.wav"};

	if (argc < 2) {
		cerr << usage << endl;
		return -1;
	}

	const string file_name {argv[1]};

	if (SDL_Init(SDL_INIT_AUDIO)) {
		SDL_Quit();		
		throw runtime_error("Could not initialize SDL audio.");
	}
	cout << "init" << endl;

	Buffer buffer;
	SDL_AudioSpec spec;

	unique_ptr<RingBuffer> ring{new RingBuffer{1048576}};
	cout << "ring" << endl;
	
	auto read_file = [&ring, &file_name]() {
		ifstream input(file_name, std::ifstream::binary);
		cout << "pre file" << endl;
		while (input) {
			cout << "file 1024" << endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			const size_t size {1024};
			uint8_t temp[size];
			input.read(reinterpret_cast<char*>(temp), size);
			ring->push_lock(temp, size);
		}
		cout << "file exit" << endl;
	};
	std::thread file_reader(read_file);


	SDL_RWops* source = SDL_AllocRW();
	source->size = size;
	source->seek = seek;
	source->read = read;
	source->close = close;
	source->type = SDL_RWOPS_UNKNOWN;
	source->hidden.unknown.data1 = ring.get();

	if (!SDL_LoadWAV_RW(source, false, &spec, 
	                 &buffer.position, &buffer.length)) {
		SDL_FreeWAV(buffer.position);
		SDL_Quit();		
	  return -1;
	}
	cout << "load wav" << endl;

	spec.callback = audio_callback;
	spec.userdata = &buffer;

	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
			nullptr, 0, &spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
	cout << "open audio" << endl;

	if (!dev) {
		SDL_Quit();		
		return -1;
	} else {
		SDL_PauseAudioDevice(dev, 0);
		file_reader.join();

		SDL_Delay(50000);
		SDL_CloseAudioDevice(dev);
		return 0;
	}

}

void audio_callback(void *userdata, Uint8 *stream, int len) {
	
	Buffer *b = reinterpret_cast<Buffer*>(userdata);

	if (b->length) {
		const auto write_length = min(b->length, static_cast<Uint32>(len));
		SDL_memcpy (stream, b->position, write_length);
		b->position += write_length;
		b->length -= write_length;
	}
}
