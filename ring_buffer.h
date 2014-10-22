#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

class RingBuffer {
protected:
	const size_t size_;
	const std::unique_ptr<uint8_t[]> buffer_;
	size_t begin_{0};
	size_t end_{0};

public:
	RingBuffer(size_t size); 
	virtual bool push(const uint8_t* input, size_t input_size);
	virtual bool pop(uint8_t* output, size_t output_size);

protected:
	size_t space_left() const;
	size_t space_used() const;
};

class RingBufferLock : public RingBuffer {
private:
	std::mutex m_;
	std::condition_variable empty_;
	std::condition_variable full_;

public:
	RingBufferLock(size_t size) : RingBuffer(size) {};
	virtual bool push(const uint8_t* input, size_t input_size) override;
	virtual bool pop(uint8_t* output, size_t output_size) override;
};
