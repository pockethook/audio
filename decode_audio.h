#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
}

#include "ring_buffer.h"

class DecodeAudio {
private:
	AVFormatContext* format_context_{nullptr};
	AVCodecContext* decoder_context_{nullptr};
	AVFrame* frame_;
	AVPacket packet_;

	RingBuffer* ring_;

public:
	DecodeAudio(const std::string &file_name, RingBuffer* ring) :
			frame_{av_frame_alloc()},
			ring_{ring} {
		av_register_all();

		if (avformat_open_input(&format_context_, file_name.c_str(),
		                        nullptr, nullptr)) {
			throw;
		}
		if (avformat_find_stream_info(format_context_, nullptr)) {
			throw;
		}
		auto stream_index =
		   	av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO,
			                    -1, -1, nullptr, 0);
		if (stream_index < 0) {
			throw;
		}
		decoder_context_ = format_context_->streams[stream_index]->codec;
		auto decoder = avcodec_find_decoder(decoder_context_->codec_id);
		if (!decoder) {
			throw;
		}
		avcodec_open2(decoder_context_, decoder, nullptr);
		av_dump_format(format_context_, 0, file_name.c_str(), 0);

		av_init_packet(&packet_);
	}

	std::string format() const {
		auto packed = av_get_packed_sample_fmt(
			static_cast<AVSampleFormat>(frame_->format));
		auto format = av_get_sample_fmt_name(packed);
		return format ? format : std::string{};
	}

	unsigned samples() const {
		return frame_->nb_samples;
	}

	unsigned sample_rate() const {
		return frame_->sample_rate;
	}

	bool operator()() {
		av_read_frame(format_context_, &packet_);
		int frame_finished;
		auto size = avcodec_decode_audio4(decoder_context_, frame_,
		                                  &frame_finished, &packet_);
		if (frame_finished) {
			unsigned channels = frame_->channels;
			size_t linesize = frame_->linesize[0];
			if (av_sample_fmt_is_planar(
					static_cast<AVSampleFormat>(frame_->format))) {
				for (unsigned i = 0; i < linesize; i += 2) {
					for (unsigned channel = 0; channel < channels; ++channel) {
						ring_->push(&frame_->extended_data[channel][i], 2);
					}
				}
			} else {
				ring_->push(&frame_->extended_data[0][0], linesize);
			}
		}
		auto decoded = std::min(size, packet_.size);
		if (decoded < 0) {
			return false;
		} else {
			packet_.data += decoded;
			packet_.size -= decoded;
			return true;
		}
	}
};

std::ostream &operator<<(std::ostream &o, DecodeAudio &decode) {
	o << "format " << decode.format() << std::endl;
	o << "samples " << decode.samples() << std::endl;
	o << "sample_rate " << decode.sample_rate() << std::endl;
	return o;
}

void decode_audio(const std::string &file_name, RingBuffer* ring);
