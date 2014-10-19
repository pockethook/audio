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

ostream &operator<<(ostream &o, const SDL_AudioSpec &spec) {
	o << "freq" << ' ' <<  spec.freq << endl;
	o << "format" <<  ' ' << spec.format << ' ' << AUDIO_S16LSB << endl;
	o << "channels" <<  ' ' << static_cast<int>(spec.channels) << endl;
	o << "samples" <<  ' ' << spec.samples << endl;
	return o;
}

void print_devices() {
	const int capture = 0;
	for (unsigned i = 0; i < SDL_GetNumAudioDevices(capture); ++i) {
		cout << "Device " << i << ": "
		     << SDL_GetAudioDeviceName(i, capture) << endl;
	}
}


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

	Buffer b;
	SDL_AudioSpec wav_spec;
	
	if (!SDL_LoadWAV(file_name.c_str(), &wav_spec, &b.position, &b.length)) {
		SDL_FreeWAV(b.position);
		SDL_Quit();		
	  return -1;
	}

	wav_spec.callback = audio_callback;
	wav_spec.userdata = &b;

	SDL_AudioSpec device_spec;
	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
			nullptr, 0, &wav_spec, &device_spec, SDL_AUDIO_ALLOW_ANY_CHANGE);

	if (!dev) {
		return -1;
	} else {
		cout << device_spec << endl;
		cout << wav_spec << endl;
		print_devices();

		const int capture = 0;
		SDL_PauseAudioDevice(dev, capture);
		SDL_Delay(50000);
		SDL_CloseAudioDevice(dev);
		return 0;
	}

}

void audio_callback(void *userdata, Uint8 *stream, int len) {
	
	Buffer *b = reinterpret_cast<Buffer*>(userdata);

	if (b->length) {
		const auto write_length = min(b->length, static_cast<Uint32>(len));
		//SDL_memcpy (stream, b->position, write_length);
		// mix from one buffer into another
		SDL_memset(stream, rand(), len);
		SDL_MixAudioFormat(
			stream, b->position, AUDIO_S16LSB,
		   	write_length, SDL_MIX_MAXVOLUME);
		
		b->position += write_length;
		b->length -= write_length;
	}
}
