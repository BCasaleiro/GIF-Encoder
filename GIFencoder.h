#ifndef _GIFENCODER_H_
#define _GIFENCODER_H_

#define MAX_COLORS 256

//#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

typedef struct dictStruct {
	int index;
	char* key;
} Dict;

typedef struct _imageStruct {
	int width;
	int height;
	char *pixels;
	char *colors;
	int numColors;
	char minCodeSize;
} imageStruct;

imageStruct* GIFEncoder(unsigned char *data, int width, int height);
void RGB2Indexed(unsigned char *data, imageStruct* image);
int nextPower2(int n);
char numBits(int n);
void GIFEncoderWrite(imageStruct* image, char* outputFile);
void writeGIFHeader(imageStruct* image, FILE* file);
//
void writeImageBlockHeader(imageStruct* image, FILE* file);
void LZWCompress(FILE* file, int minCodeSize, char* pixels, int npixels, int ncolors);
//Auxiliares
Dict* initDict(int size);
void fillDict(Dict* dict, int *dictPos, int ncolors);
int searchInDict(Dict* dict, int dictPos, char* key);
void insertInDict(Dict *dict, int dictPos, char* key);
Dict* doubleDictSpace(Dict *dict, int dictSize);

void printDict(Dict *dict, int dictPos);

int ndigits(int n);

#endif
