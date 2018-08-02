#ifndef CONFIGS_H
#define CONFIGS_H

#include <stdint.h>

#define SAMPLE_RATE 44100
#define BIT_TIME 5
#define MAX_VOLUME 0x5050

#define LOW_FREQUENCY BIT_TIME * 500
#define LOW_LENGTH 2
#define MEDIUM_LEGNTH 4
#define HIGH_LENGTH 8
//const int SAMPLES_PER_BIT = SAMPLES_PER_MS * BIT_TIME;
#define SAMPLES_PER_BIT SAMPLE_RATE / 1000 * BIT_TIME

enum SignalType {
	NewData,
	EndData,
	Bit0,
	Bit1,
};

struct SignalStream {
	enum SignalType* signals;
	int size;
};

struct SoundStream {
	int16_t* data;
	uint32_t size;
};

struct FmtHeader {
	char id[4];
	uint32_t size;
	uint16_t format;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};

struct WavFile {
	char id[4];
	uint32_t file_size;
	char format[4];
	struct FmtHeader fmt_header;
	char data_header[4];
	uint32_t data_samples;
};

#endif
