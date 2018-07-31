#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define BIT_TIME 4
#define MAX_VOLUME 0x7070

const int LOW_FRE = BIT_TIME * 500;
const int HIGH_FRE = BIT_TIME * 1000;
const int SAMPLES_PER_MS = SAMPLE_RATE / 100;
const int SAMPLES_PER_BIT = SAMPLES_PER_MS * BIT_TIME;

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

void copy_samples(int16_t* src, int16_t* dst, int pos, int qnt) {
	for(int i = 0; i < qnt; ++i, ++pos) {
		dst[pos] = src[i];
	}
}

int16_t* generate_single_wave(int samples_per_fase) {
	int16_t* data = (int16_t*) malloc(samples_per_fase * 2);

	for(int i = 0; i < samples_per_fase; ++i) {
		data[i] = MAX_VOLUME;
		data[samples_per_fase+i] = -MAX_VOLUME;
	}

	return data;
}

int16_t* generate_square(int frequency) {
	int16_t* data = (int16_t*) malloc(SAMPLES_PER_BIT * 2);
	
	float fase_time = 1/((float) frequency * 2) * 1000;
	int samples_per_fase = (int)(fase_time * SAMPLES_PER_MS);
	int total_waves = (int)((fase_time*SAMPLES_PER_MS)/(float)BIT_TIME);
	printf("fase_time: %f ms\n", fase_time);
	printf("samples_per_fase: %d\n", samples_per_fase);
	printf("total_waves: %d\n", total_waves);
	printf("total_samples: %d\n", total_waves * samples_per_fase*2);

	int s = 0;
	for(int i = 0; i < total_waves; ++i) {
		for(int j = 0; j < samples_per_fase; ++j) {
			data[samples_per_fase*i + j] = MAX_VOLUME;
			++s;
		}
		printf("\twave #%d\n", i);
	}

	printf("s: %d - %d\n", s, SAMPLES_PER_BIT);
	printf("\n");
	return data;
}

int16_t* generate_sine(int frequency) {
	int16_t* data = (int16_t*) malloc(SAMPLES_PER_BIT * 2);

	for(int i = 0; i < SAMPLES_PER_BIT; ++i) {
		double v0 = sin((M_PI * i * frequency) / SAMPLE_RATE);
		data[i] = (int16_t) (v0 * 0x7070);
	}

	return data;
}

int16_t* generate_low() {
	return generate_square(LOW_FRE);
}

int16_t* generate_high() {
	return generate_square(HIGH_FRE);
}

void copy_bits(int16_t* src, int16_t* dst, int pos, int qnt) {
	for(int i = 0; i < SAMPLES_PER_BIT * qnt; ++i, ++pos) {
		dst[pos] = src[i];
	}
}

void print_bits(int16_t* data, int size) {
	for(int i = 0; i < size; ++i) {
		printf("#%08x - %08x\n", i, data[i]);
	}
}

int16_t* encode_bit(bool value, int16_t* low, int16_t* high) {
	int16_t* data = (int16_t*) malloc(SAMPLES_PER_BIT * 4);

	if(value) {
		copy_bits(high, data, 0, 1);
		copy_bits(low, data, SAMPLES_PER_BIT, 1);
	} else {
		copy_bits(low, data, 0, 1);
		copy_bits(high, data, SAMPLES_PER_BIT, 1);
	}

	return data;
}

int16_t* encode_char(char c, int16_t* low, int16_t* high) {
	int16_t* data = (int16_t*) malloc(SAMPLES_PER_BIT * 32);
	
	for(int i = 0, b = 0x01, pos = 0; i < 8; ++i) {
		int16_t* samples = encode_bit((c&b)==b, low, high);
		copy_bits(samples, data, pos, 2);

		free(samples);
		pos += SAMPLES_PER_BIT * 2;
		b = b << 1;
	}

	return data;
}

int16_t* generate_data(char* string, uint32_t samples) {
	int16_t* low = generate_low();
	int16_t* high = generate_high();
	int16_t* data = (int16_t*) malloc(samples * 2);
	
	for(size_t i = 0, pos = 0; i < strlen(string); ++i) {
		int16_t* samples = encode_char(string[i], low, high);
		copy_bits(samples, data, pos, 16);

		free(samples);
		pos += SAMPLES_PER_BIT * 16;
	}
	
	free(low);
	free(high);
	return data;
}


int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("bts <file-output> <string>\n");
		return 1;
	}

	FILE* fl;
	fl = fopen(argv[1], "wb");
	if(!fl) {
		printf("Nao pude abrir o arquivo\n");
		return 1;
	}
	
	uint32_t samples = strlen(argv[2]) * 16 * BIT_TIME * SAMPLES_PER_MS;
	int16_t* data = generate_data(argv[2], samples);

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
		36 + samples * 2,
		{'W', 'A', 'V', 'E'},
		fmt_header,
		{'d', 'a', 't', 'a'},
		samples * 2
	};

	fwrite(&wave_file, sizeof(wave_file), 1, fl);

	for(uint32_t i = 0; i < samples; ++i) {
		fwrite(&data[i], 2, 1, fl);
	}

	free(data);
	fclose(fl);
	return 0;
}
