#include "decode_audio.h"
#include "ring_buffer.h"

#include <string>
#include <iostream>
#include <algorithm>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
}

using std::min;
using std::copy;

void decode_audio(const std::string &file_name, RingBuffer* ring) {
	DecodeAudio decode(file_name, ring);
	while (decode()) {
		std::cout << decode << std::endl;
	}
}
