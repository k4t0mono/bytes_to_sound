// Encode byte stream in a wav file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "configs.h"

void inspect_signals(enum SignalType* signals, int size) {
	for(int i = 0; i < size; ++i) {
		switch(signals[i]) {
			case NewData:
				printf("N "); break;

			case EndData:
				printf("E "); break;

			case Bit0:
				printf("0 "); break;

			case Bit1:
				printf("1 "); break;

			default:
				printf("x "); break;
		}
	}
	printf("\n");
}

enum SignalType* byte_to_signal(char c) {
	enum SignalType* signals = (enum SignalType*) malloc(9 * sizeof(enum SignalType));

	bool checksum = false;
	for(int i = 0, b = 0x01; i < 8; ++i) {
		bool result = (c&b) == b;
		signals[i] = result ? Bit1 : Bit0;

		checksum = checksum ^ result;
		b =  b << 1;
	}
	signals[8] = checksum ? Bit1 : Bit0 ;

	return signals;
}

struct SignalStream* gen_signal_stream(char* data) {
	struct SignalStream* ss = (struct SignalStream*) malloc(sizeof(struct SignalStream));
	ss->size = strlen(data) * 9 + 2;
	ss->signals = (enum SignalType*) malloc(ss->size * sizeof(enum SignalType));

	ss->signals[0] = NewData;
	ss->signals[ss->size-1] = EndData;

	for(size_t i = 0; i < strlen(data); ++i) {
		enum SignalType* s = byte_to_signal(data[i]);
		for(size_t j = 0; j < 9; ++j) {
			ss->signals[i*9+j+1] = s[j];
		}
		free(s);
	}

	return ss;
}

int16_t* gen_pulse(int num) {
	int16_t* samples = (int16_t*) malloc(sizeof(int16_t) * SAMPLES_PER_BIT);
	if(num == 0) { return samples; }
	
	int samples_per_wave = SAMPLES_PER_BIT / num;
	int samples_per_fase = samples_per_wave / 2;
	for(int i = 0; i < num; ++i) {
		for(int j = 0; j < samples_per_fase; ++j) {
			samples[i*samples_per_wave + j] = MAX_VOLUME;
			samples[i*samples_per_wave + samples_per_fase+j] = -MAX_VOLUME;
		}
	}

	return samples;
}

void copy_bits(int16_t* src, int16_t* dst, int pos, int n) {
	for(int i = 0; i < SAMPLES_PER_BIT*n; ++i) {
		dst[pos+i] = src[i];
	}
}

int16_t* signal_to_wave(enum SignalType st) {
	int16_t* samples = (int16_t*) malloc(sizeof(int16_t) * 2 * SAMPLES_PER_BIT);
	int16_t* p0 = NULL;
	int16_t* p1 = NULL;
	switch(st) {
		case NewData:
			p0 = gen_pulse(LOW_LENGTH);
			p1 = gen_pulse(HIGH_LENGTH);
			break;

		case EndData:
			p0 = gen_pulse(HIGH_LENGTH);
			p1 = gen_pulse(LOW_LENGTH);
			break;

		case Bit0:
			p0 = gen_pulse(MEDIUM_LEGNTH);
			p1 = gen_pulse(LOW_LENGTH);
			break;

		case Bit1:
			p0 = gen_pulse(LOW_LENGTH);
			p1 = gen_pulse(MEDIUM_LEGNTH);
			break;

		default:
			p0 = gen_pulse(0);
			p1 = gen_pulse(0);
			break;
	}

	copy_bits(p0, samples, 0, 1);
	copy_bits(p1, samples, SAMPLES_PER_BIT, 1);
	free(p0);
	free(p1);
	return samples;
}

struct SoundStream* gen_samples(struct SignalStream* ss) {
	struct SoundStream* sound_stream = (struct SoundStream*) malloc(sizeof(struct SoundStream));
	sound_stream->size = ss->size * 2 * SAMPLES_PER_BIT;
	sound_stream->data = (int16_t*) malloc(sizeof(int16_t) * sound_stream->size);

	for(int i = 0; i < ss->size; ++i) {
		int16_t* samples = signal_to_wave(ss->signals[i]);
		copy_bits(samples, sound_stream->data, i*SAMPLES_PER_BIT*2, 2);
		free(samples);
	}

	return sound_stream;
}


void encode_bytes(char* data, char* file_name) {
	struct SignalStream* ss = gen_signal_stream(data);
	struct SoundStream* samples = gen_samples(ss);


	struct FmtHeader fmt_header = {
		{'f', 'm', 't', ' '},	/* Default ID */
		16,						/* Size for PCM */
		1,						/* PCM format */
		1,						/* Mono */
		SAMPLE_RATE,			
		176400,					
		2,						/* 2 block per sample */
		16						/* Saples of 16 bits */
	};

	struct WavFile wave_file = {
		{'R', 'I', 'F', 'F'},
		36 + samples->size * 2,
		{'W', 'A', 'V', 'E'},
		fmt_header,
		{'d', 'a', 't', 'a'},
		samples->size * 2
	};

	FILE* fl;
	fl = fopen(file_name, "wb");
	if(!fl) {
		exit(1);
	}

	fwrite(&wave_file, sizeof(wave_file), 1, fl);
	for(uint32_t i = 0; i < samples->size; ++i) {
		fwrite(&samples->data[i], 2, 1, fl);
	}

	fclose(fl);

	free(samples->data);
	free(samples);
	free(ss->signals);
	free(ss);
}
