#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define MILISEC 441
#define BIT_TIME 10
#define LOW_FRE 4400
#define HIGH_FRE 2200


void write_header(FILE* fl, int file_size) {
	int num;

	fwrite("RIFF", 1, 4, fl);		// Chunk ID
	fwrite(&file_size, 4, 1, fl);	// File size
	fwrite("WAVE", 1, 4, fl);		// Format
	fwrite("fmt ", 1, 4, fl);		// fmt header

	num = 16;						// Size of fmt header 
	fwrite(&num, 4, 1, fl);

	num = 1;						// PCM
	fwrite(&num, 2, 1, fl);

	num = 1;						// Mono
	fwrite(&num, 2, 1, fl);

	num = SAMPLE_RATE;				// Sample rate
	fwrite(&num, 4, 1, fl);

	num = 176400;					// Byte rate
	fwrite(&num, 4, 1, fl);

	num = 2;						// Block align
	fwrite(&num, 2, 1, fl);
	
	num = 16;						// Bits per sample
	fwrite(&num, 2, 1, fl);
}


void write_low(int ms, FILE* fl) {
	for(int i = 0; i < MILISEC*ms; ++i) {
		double v0 = sin((M_PI * i * LOW_FRE) / SAMPLE_RATE);
		int16_t sample = (int16_t) (v0 * 0x7070);
		fwrite(&sample, 2, 1, fl);
	}
}


void write_high(int ms, FILE* fl) {
	for(int i = 0; i < MILISEC*ms; ++i) {
		double v0 = sin((M_PI * i * HIGH_FRE) / SAMPLE_RATE);
		int16_t sample = (int16_t) (v0 * 0x7070);
		fwrite(&sample, 2, 1, fl);
	}
}


void write_manchester(bool r, int ms, FILE* fl) {
	if(r) {
		write_high(ms/2, fl);
		write_low(ms/2, fl);
	} else {
		write_low(ms/2, fl);
		write_high(ms/2, fl);
	}
}


void write_char(char c, int ms, FILE* fl) {
	printf("%02x", c);
	write_manchester((c&0x01) == 0x01, ms, fl);
	write_manchester((c&0x02) == 0x02, ms, fl);
	write_manchester((c&0x04) == 0x04, ms, fl);
	write_manchester((c&0x08) == 0x08, ms, fl);
	write_manchester((c&0x10) == 0x10, ms, fl);
	write_manchester((c&0x20) == 0x20, ms, fl);
	write_manchester((c&0x40) == 0x40, ms, fl);
	write_manchester((c&0x80) == 0x80, ms, fl);
}


void write_string(char* str, int len, FILE* fl) {
	write_char(0x7e, BIT_TIME, fl);
	for(int i = 0; i < len; ++i) {
		write_char(str[i], BIT_TIME, fl);
	}
	printf("\n");
	write_char(0x7e, BIT_TIME, fl);
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("bts <file-output> <string>");
	}

	FILE* fl;
	fl = fopen(argv[1], "wb");
	if(!fl) {
		printf("Nao pude abrir o arquivo\n");
		return 1;
	}
	
	//int numSamples = SAMPLE_RATE * 2 * (BIT_TIME * 8)/100 * (strlen(argv[2]) + 2);
	int numSamples = 0;
	write_header(fl, 36 + numSamples);

	fwrite("data", 1, 4, fl);
	fwrite(&numSamples, 4, 1, fl);
	
	//write_string(argv[2], strlen(argv[2]), fl);

	fclose(fl);
	return(0);
}
