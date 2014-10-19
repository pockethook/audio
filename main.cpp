#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::min;

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
		return -1;
	}

	Buffer buffer;
	SDL_AudioSpec spec;
	
	if (!SDL_LoadWAV(file_name.c_str(), &spec,
	                 &buffer.position, &buffer.length)) {
		SDL_FreeWAV(buffer.position);
		SDL_Quit();		
	  return -1;
	}

	spec.callback = audio_callback;
	spec.userdata = &buffer;

	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
			nullptr, 0, &spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);

	if (!dev) {
		SDL_Quit();		
		return -1;
	} else {
		SDL_PauseAudioDevice(dev, 0);
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
