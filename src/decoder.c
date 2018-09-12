#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "configs.h"
#include "decoder.h"


enum SignalType decode_signal(int16_t* samples, int pos) {
	int num_waves_0 = 0;
	int num_waves_1 = 0;
	
	for(int i = 1; i < SAMPLES_PER_BIT; ++i) {
		int j = pos*2*SAMPLES_PER_BIT + i;
		int k = j + SAMPLES_PER_BIT;

		if(samples[j-1] == MAX_VOLUME && samples[j-1] != samples[j]) {
			num_waves_0++;
		}

		if(samples[k-1] == MAX_VOLUME && samples[k-1] != samples[k]) {
			num_waves_1++;
		}
	}

	if(num_waves_0 == LOW_LENGTH) {
		if(num_waves_1 == HIGH_LENGTH) { return NewData; }
		if(num_waves_1 == MEDIUM_LEGNTH) { return Bit1; }

	} else if(num_waves_0 == MEDIUM_LEGNTH) {
		if(num_waves_1 == LOW_LENGTH) { return Bit0; }

	} else if(num_waves_0 == HIGH_LENGTH) {
		if(num_waves_1 == LOW_LENGTH) { return EndData; }
	}

	fprintf(stderr, "Error: Invalid encoding at signal #%d\n", pos);
	exit(10);
}

char decode_byte(int16_t* samples, int pos) {
	bool byte[9];
	for(int i = 0; i < 9; ++i) {
		byte[i] = decode_signal(samples, pos*9+i+1) == Bit1;
	}

	char c = 0x00;
	bool checksum = false;
	for(int i = 0, b = 0x01; i < 8; ++i, b = b << 1) {
		c = byte[i] ? c | b : c;
		checksum = checksum ^ byte[i];
	}
	if(checksum != byte[8]) {
		fprintf(stderr, "Erro: Invalid checksum\n");
		exit(11);
	}

	return c;
}

char* decode_pulses(int16_t* samples) {
	enum SignalType st;

	st = decode_signal(samples, 0);
	if(st != NewData) {
		fprintf(stderr, "Error: Invalid encoding at signal #%d\n", 0);
		exit(10);
	}

	st = decode_signal(samples, 1);
	if(st == EndData) {
		printf("Dado vazio\n");
		return NULL;
	}

	char* data = (char*) malloc(0);
	for(int i = 0; st != EndData; ++i) {
		data = (char*) realloc(data, (i+1) * sizeof(char));
		data[i] = decode_byte(samples, i);

		st = decode_signal(samples, (i+1)*9 + 1);
	}

	return data;
}

void decode_wav(char* wave_file, char* output_file) {
	FILE* fl;
	fl = fopen(wave_file, "r");

	struct WavFile wav_header;
	fread(&wav_header, 1, sizeof(struct WavFile), fl);

	uint32_t num_samples = wav_header.data_samples/2;

	int16_t samples[num_samples];
	for(uint32_t i = 0; i < num_samples; ++i) {
		fread(&samples[i], 1, sizeof(int16_t), fl);
	}

	char* data = decode_pulses(samples);
	fclose(fl);

	fl = fopen(output_file, "wb");
	fwrite(data, strlen(data), 1, fl);

	fclose(fl);
	free(data);
}
