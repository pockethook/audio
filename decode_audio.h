#pragma once

#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "ring_buffer.h"

void decode_audio(const std::string &file_name, RingBuffer* ring);
