#ifndef _BITFILE_H_
#define _BITFILE_H_

#include "stdio.h"

typedef struct _bitStream {
	FILE* output_;
	char* buffer_;
	int index_, bitsLeft_;
} bitStream;

bitStream* bitFile(FILE* file);
void flush(bitStream* stream);
void writeBits(bitStream* stream, int bits, int numbits);

#endif