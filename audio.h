#pragma once

#include "ring_buffer.h"

#include <functional>
#include <string>
#include <unordered_map>

const std::unordered_map<std::string, unsigned> format_sizes {
	{"u8", sizeof(uint8_t)},
	{"s16", sizeof(int16_t)},
	{"s32", sizeof(int32_t)},
	{"flt", sizeof(float)},
	{"dbl", sizeof(double)},
};

class Audio {
protected:
	RingBuffer* ring_;

public:
	Audio(RingBuffer* const ring) : ring_{ring} {};
	virtual ~Audio() {};
	virtual void operator()() = 0;
};
