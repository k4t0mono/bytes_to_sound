#include <stdio.h>
#include "encoder.h"

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("bts <string> <file-output>");
	}

	encode_bytes(argv[1], argv[2]);

	return(0);
}
