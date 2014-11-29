
#include "BitFile.h"

#include "stdlib.h"
#include "string.h"

bitStream* bitFile(FILE* file) {
	int i;
	bitStream* stream = (bitStream*)malloc(sizeof(bitStream));
	stream->output_ = file;
	stream->buffer_ = (char*)calloc(256, sizeof(char));
	for (i=0; i<256; i++)
		stream->buffer_[i] = (char)0;
	stream->index_ = 0;
	stream->bitsLeft_ = 8;
	return stream;
}

void flush(bitStream* stream) {
	int i;
	int numBytes = stream->index_ + (stream->bitsLeft_ == 8 ? 0 : 1);
	if (numBytes > 0) {
		fprintf(stream->output_, "%c", (char)numBytes);
		for (i=0; i<numBytes; i++)
			fprintf(stream->output_, "%c", (char)stream->buffer_[i]);
		stream->buffer_[0] = 0;
		stream->index_ = 0;
		stream->bitsLeft_ = 8;
	}
}

void writeBits(bitStream* stream, int bits, int numbits) {
	int i, bitsWritten = 0;
	int numBytes = 255;
	FILE* fp = fopen("test.txt", "a");

	do {
		if ((stream->index_ == 254 && stream->bitsLeft_ == 0) || stream->index_ > 254) {
			fprintf(stream->output_, "%c", (char)numBytes);

			for (i = 0; i < numBytes; i++)
				fprintf(stream->output_, "%c", stream->buffer_[i]);

			for (i = 0; i <= numBytes; i++)
				fprintf(fp, "%d: %d\n", i + 1, stream->buffer_[i]);

			stream->buffer_[0] = 0;
			stream->index_ = 0;
			stream->bitsLeft_ = 8;
		}

		if (numbits <= stream->bitsLeft_) {
			stream->buffer_[stream->index_] |= (bits & ((1 << numbits) - 1)) << (8 - stream->bitsLeft_);
			bitsWritten += numbits;
			stream->bitsLeft_ -= numbits;
			numbits = 0;
		} else {
			stream->buffer_[stream->index_] |= (bits & ((1 << stream->bitsLeft_) - 1)) << (8 - stream->bitsLeft_);
			bitsWritten += stream->bitsLeft_;
			bits >>= stream->bitsLeft_;
			numbits -= stream->bitsLeft_;
			stream->buffer_[++stream->index_] = 0;
			stream->bitsLeft_ = 8;
		}
	} while (numbits != 0);
	fclose(fp);
}
