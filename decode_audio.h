#pragma once

#include "ring_buffer.h"

extern "C" {
#include <libavformat/avformat.h>
}

class DecodeAudio {
private:
	AVFormatContext* format_context_{nullptr};
	AVCodecContext* decoder_context_{nullptr};
	AVFrame* frame_;
	AVPacket packet_;
	int stream_index_{-1};

	RingBuffer* ring_;

public:
	DecodeAudio(const std::string &file_name, RingBuffer* ring);

	unsigned sample_rate() const;
	std::string format() const;
	unsigned channels() const;
	unsigned samples() const;
	bool is_planar() const;
	unsigned bytes() const;

	bool operator()();

private:
	bool next_audio_packet();
	bool decode_audio_packet();
	void push_audio_frame();
};

std::ostream &operator<<(std::ostream &o, DecodeAudio &decode);
