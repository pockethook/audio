#include "ring_buffer.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

using std::min;
using std::max;
using std::mutex;
using std::runtime_error;
using std::unique_lock;
using std::unique_ptr;
using std::copy;

RingBuffer::RingBuffer(const size_t size) :
		size_{size},
		buffer_{new uint8_t[size_]},
		begin_{0},
		end_{0} {
}

// Return false if the requested input_size is too large
// Return true if input has successfully been pushed into the buffer
// Do not overwrite unused data
bool RingBuffer::push(const uint8_t* const input, const size_t input_size) {
	if (input_size >= space_left()) {
		return false;
	}

	auto split = min(size_ - end_, input_size);	
	copy(&input[0], &input[split], &buffer_[end_]);
	copy(&input[split], &input[input_size], &buffer_[0]);

	end_ = (end_ + input_size) % size_;
	return true;
}

// Return false if the requested output_size is too large
bool RingBuffer::pop(uint8_t* const output, const size_t output_size) {
	if (output_size > space_used()) {
		return false;
	}

	auto split = min(size_ - begin_, output_size);	
	copy(&buffer_[begin_], &buffer_[begin_ + split], &output[0]);
	copy(&buffer_[0], &buffer_[output_size - split], &output[split]);

	begin_ = (begin_ + output_size) % size_;
	return true;
}

// This is a lower bound as begin_ may move
size_t RingBuffer::space_left() const {
	return size_ - space_used();
}

// This is a lower bound as end_ may move
size_t RingBuffer::space_used() const {
	return (size_ + end_ - begin_) % size_;
}

bool RingBufferLock::push(const uint8_t* const input,
                           const size_t input_size) {
	unique_lock<mutex> lock(m_);

	for (;;) {
		if (finished_) {
			return false;
		} else if (input_size < space_left()) {
			auto split = min(size_ - end_, input_size);	
			copy(&input[0], &input[split], &buffer_[end_]);
			copy(&input[split], &input[input_size], &buffer_[0]);

			end_ = (end_ + input_size) % size_;
			empty_.notify_one();
			break;
		} else {
			full_.wait(lock);
		}
	}
	return true;
}

bool RingBufferLock::pop(uint8_t* const output, const size_t output_size) {
	unique_lock<mutex> lock(m_);

	for (;;) {
		if (output_size <= space_used()) {
			auto split = min(size_ - begin_, output_size);	
			copy(&buffer_[begin_], &buffer_[begin_ + split], &output[0]);
			copy(&buffer_[0], &buffer_[output_size - split], &output[split]);

			begin_ = (begin_ + output_size) % size_;
			full_.notify_one();
			break;
		} else if (finished_) {
			return false;
		} else {
			empty_.wait(lock);
		}
	}
	return true;
}
