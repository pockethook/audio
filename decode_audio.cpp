#include "decode_audio.h"

#include "ring_buffer.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
}

using std::endl;
using std::min;
using std::ostream;
using std::runtime_error;
using std::string;

DecodeAudio::DecodeAudio(const string &file_name, RingBuffer* ring) :
		frame_{av_frame_alloc()},
		ring_{ring} {
	av_register_all();

	if (avformat_open_input(&format_context_, file_name.c_str(),
							nullptr, nullptr)) {
		throw runtime_error("Could not open input file.");
	}
	if (avformat_find_stream_info(format_context_, nullptr)) {
		throw runtime_error("Could not find stream information");
	}
	stream_index_ =
		av_find_best_stream(format_context_, AVMEDIA_TYPE_AUDIO,
							-1, -1, nullptr, 0);
	if (stream_index_< 0) {
		throw runtime_error("Could not find audio stream");
	}
	decoder_context_ = format_context_->streams[stream_index_]->codec;
	auto decoder = avcodec_find_decoder(decoder_context_->codec_id);
	if (!decoder) {
		throw runtime_error("Could not find audio decoder");
	}
	avcodec_open2(decoder_context_, decoder, nullptr);
	av_dump_format(format_context_, 0, file_name.c_str(), 0);

	av_init_packet(&packet_);
}

unsigned DecodeAudio::sample_rate() const {
	return decoder_context_->sample_rate;
	//return frame_->sample_rate;
}

string DecodeAudio::format() const {
	auto packed = av_get_packed_sample_fmt(
		static_cast<AVSampleFormat>(decoder_context_->sample_fmt));
	auto format = av_get_sample_fmt_name(packed);
	return format ? format : string{};
	//auto packed = av_get_packed_sample_fmt(
	//	static_cast<AVSampleFormat>(frame_->format));
	//auto format = av_get_sample_fmt_name(packed);
	//return format ? format : string{};
}

unsigned DecodeAudio::channels() const {
	return decoder_context_->channels;
	//return frame_->channels;
}

unsigned DecodeAudio::samples() const {
	return 8192;
	//return frame_->nb_samples;
}

bool DecodeAudio::is_planar() const {
	return av_sample_fmt_is_planar(
		static_cast<AVSampleFormat>(decoder_context_->sample_fmt));
	//return av_sample_fmt_is_planar(
	//	static_cast<AVSampleFormat>(frame_->format));
}

unsigned DecodeAudio::bytes() const {
	return av_get_bytes_per_sample(
		static_cast<AVSampleFormat>(decoder_context_->sample_fmt));
	//return av_get_bytes_per_sample(
	//	static_cast<AVSampleFormat>(frame_->format));
}

// Return true if a whole frame has been successfully decoded and pushed
// Return false if EOF
// Throw if decoding fails
bool DecodeAudio::operator()() {
	for (;;) {
		if (!next_audio_packet()) {
			ring_->finished();
			return false;
		}

		// Decode packets until a whole frame has been produced
		if (decode_audio_packet()) {
			break;
		}
	}

	push_audio_frame();
	return true;
}

// Return true if there is a new audio packet to be processed
// Return false if we have reached the end of the file
bool DecodeAudio::next_audio_packet() {
	for (;;) {
		if (av_read_frame(format_context_, &packet_) < 0) {
			return false;
		}
		if (packet_.stream_index == stream_index_) {
			break;
		} else {
			av_free_packet(&packet_);
		}
	}
	return true;
}

// Return true if a whole frame has been decoded
// Return false if only a partial frame has been decoded
bool DecodeAudio::decode_audio_packet() {
	int frame_finished;
	while (packet_.size) {
		auto size = avcodec_decode_audio4(
			decoder_context_, frame_, &frame_finished, &packet_);
		if (size < 0) {
			av_free_packet(&packet_);
			throw runtime_error("Error decoding audio.");
		} 

		auto decoded = min(size, packet_.size);
		packet_.data += decoded;
		packet_.size -= decoded;
	}

	av_free_packet(&packet_);

	return frame_finished;
}

void DecodeAudio::push_audio_frame() {
	size_t channels = frame_->channels;
	auto format = static_cast<AVSampleFormat>(frame_->format);
	size_t unpadded_linesize =
	   	frame_->nb_samples * av_get_bytes_per_sample(format);
	size_t bytes = av_get_bytes_per_sample(format);
	// Pack data
	if (av_sample_fmt_is_planar(format)) {
		for (size_t i = 0; i < unpadded_linesize; i += bytes) {
			for (size_t channel = 0; channel < channels; ++channel) {
				ring_->push(&frame_->extended_data[channel][i], bytes);
			}
		}
	} else {
		ring_->push(
			&frame_->extended_data[0][0], channels * unpadded_linesize);
	}
}

ostream &operator<<(ostream &o, DecodeAudio &decode) {
	o << "sample_rate " << decode.sample_rate() << endl;
	o << "format " << decode.format() << endl;
	o << "channels " << decode.channels() << endl;
	o << "samples " << decode.samples() << endl;
	o << "is_planar " << decode.is_planar() << endl;
	o << "bytes " << decode.bytes() << endl;
	return o;
}
