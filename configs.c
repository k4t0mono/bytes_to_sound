#include <stdio.h>

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
