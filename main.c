#include <stdio.h>
#include <string.h>

#include "encoder.h"
#include "decoder.h"


int main(int argc, char** argv) {
	if(argc != 4) {
		printf("bts encode <string> <file-output>\n");
		printf("bts decode <file-input> <file-output>\n");
	}

	if(!strcmp(argv[1], "encode")) {
		encode_bytes(argv[2], argv[3]);

	} else if(!strcmp(argv[1], "decode")) {
		decode_wav(argv[2], argv[3]);

	} else {
		printf("bts encode <string> <file-output>\n");
		printf("bts decode <file-input> <file-output>\n");
	}

	return(0);
}
