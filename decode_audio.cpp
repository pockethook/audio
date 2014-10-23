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
	av_register_all();
	AVFormatContext* format_context{nullptr};

	avformat_open_input(&format_context, file_name.c_str(), nullptr, nullptr);
	avformat_find_stream_info(format_context, nullptr);
	auto stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO,
	                                        -1, -1, nullptr, 0);
	auto stream = format_context->streams[stream_index];
	auto decoder_context = stream->codec;
	auto decoder = avcodec_find_decoder(decoder_context->codec_id);
	avcodec_open2(decoder_context, decoder, nullptr);
	av_dump_format(format_context, 0, file_name.c_str(), 0);

	auto frame = av_frame_alloc();
	AVPacket packet;
	av_init_packet(&packet);

	for (;;) {
		uint8_t output[1048576];
		av_read_frame(format_context, &packet);
		int frame_finished;
		auto size = avcodec_decode_audio4(decoder_context, frame, &frame_finished, &packet);
		auto decoded = min(size, packet.size);
		if (frame_finished) {
			auto unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
		copy(frame->extended_data[0],
		     frame->extended_data[0] + frame->linesize[0],
		     &output[0 * frame->linesize[0]]);
		copy(frame->extended_data[1],
		     frame->extended_data[1] + frame->linesize[0],
		     &output[1 * frame->linesize[0]]);
		//std::cout << ((AVSampleFormat)frame->format == AV_SAMPLE_FMT_S16P) << std::endl;
		//copy(frame->extended_data[0],
		//     frame->extended_data[0] + unpadded_linesize,
		//     &output[0]);
		//copy(frame->extended_data[1],
		//     frame->extended_data[1] + unpadded_linesize,
		//     &output[unpadded_linesize]);
		std::cout << decoded << ' ' << frame->linesize[0] << ' ' << unpadded_linesize << std::endl;
		for (unsigned i = 0; i < static_cast<unsigned>(frame->linesize[0]); i += 2) {
			ring->push(&output[i], 2);
			int j = 0;
			ring->push((uint8_t*)&j, 2);
		}
		//copy(frame->extended_data[0],
		//     frame->extended_data[0] + 2 * unpadded_linesize,
		//     &output[0]);
		//ring->push(&output[0], 2 * unpadded_linesize);
		}
		packet.data += decoded;
		packet.size -= decoded;
	}

}
