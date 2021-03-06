#include "decode_audio.h"
#include "ring_buffer.h"
#include "audio_factory.h"

#include <memory>
#include <string>

using std::cerr;
using std::endl;
using std::string;
using std::unique_ptr;
using std::exception;

int main(const int argc, char** argv) {

	try {
		if (argc < 2) {
			cerr << "Usage: player file_name [backend]" << endl;
			return -1;
		}

		const size_t buffer_size{1048576};
		unique_ptr<RingBuffer> ring_buffer{new RingBufferLock{buffer_size}};

		const string file_name{argv[1]};
		DecodeAudio decode(file_name, ring_buffer.get());
		// Decode a frame to get information about output format
		//decode();

		string backend = argc > 2  ? argv[2] : "alsa";
		unique_ptr<Audio> audio{make_audio(
			backend, decode.sample_rate(), decode.format(),
			decode.channels(), decode.samples(), ring_buffer.get())};
		(*audio)();

		while (decode()) {
		}

	} catch (exception &e) {
		cerr << e.what() << endl;
		return -1;
	}

	return 0;
}
