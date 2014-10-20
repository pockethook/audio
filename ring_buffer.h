#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

class RingBuffer {
private:
	const size_t size_;
	const std::unique_ptr<uint8_t[]> buffer_;
	size_t begin_{0};
	size_t end_{0};

public:
	RingBuffer(size_t size); 
	bool push(const uint8_t* input, size_t input_size);
	bool pop(uint8_t* output, size_t output_size);

private:
	size_t space_left() const;
	size_t space_used() const;
};
