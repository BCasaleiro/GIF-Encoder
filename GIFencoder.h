#ifndef _GIFENCODER_H_
#define _GIFENCODER_H_

#define MAX_COLORS 256

#include "stdio.h"
#include "string.h"
#include "math.h"

typedef struct _imageStruct {
	int width;
	int height;
	char *pixels;
	char *colors;
	int numColors;
	char minCodeSize;
} imageStruct;


//Meta Final
typedef struct element{
	int index;
	char *key;
}Dict;

int tamDict;
//

imageStruct* GIFEncoder(unsigned char *data, int width, int height);
void RGB2Indexed(unsigned char *data, imageStruct* image);
int nextPower2(int n);
char numBits(int n);
void GIFEncoderWrite(imageStruct* image, char* outputFile);
void writeGIFHeader(imageStruct* image, FILE* file);
void writeImageBlockHeader(imageStruct* image, FILE* file);

//Meta 1
void writeImageBlockHeader(imageStruct* image, FILE* file);

//Meta Final
void LZWCompress(FILE *file, imageStruct* image);
Dict* init_dict(int size_dict);
void insert_element(Dict* dict, char* key);
Dict* double_space(Dict *dict, int size_dict);
Dict* search_element(Dict* dict, char* key);

#endif
