#include "GIFencoder.h"

#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// conversão de um objecto do tipo Image numa imagem indexada
imageStruct* GIFEncoder(unsigned char *data, int width, int height) {

	imageStruct* image = (imageStruct*)malloc(sizeof(imageStruct));
	image->width = width;
	image->height = height;

	//converter para imagem indexada
	RGB2Indexed(data, image);

	return image;
}

//conversão de lista RGB para indexada: máximo de 256 cores
void RGB2Indexed(unsigned char *data, imageStruct* image) {
	int x, y, colorIndex, colorNum = 0;
	char *copy;

	image->pixels = (char*)calloc(image->width*image->height, sizeof(char));
	image->colors = (char*)calloc(MAX_COLORS * 3, sizeof(char));


	for (x = 0; x < image->width; x++) {
		for (y = 0; y < image->height; y++) {
			for (colorIndex = 0; colorIndex < colorNum; colorIndex++) {
				if (image->colors[colorIndex * 3] == (char)data[(y * image->width + x)*3] &&
					image->colors[colorIndex * 3 + 1] == (char)data[(y * image->width + x)*3 + 1] &&
					image->colors[colorIndex * 3 + 2] == (char)data[(y * image->width + x)*3 + 2])
					break;
			}

			if (colorIndex >= MAX_COLORS) {
				printf("Demasiadas cores...\n");
				exit(1);
			}

			image->pixels[y * image->width + x] = (char)colorIndex;

			if (colorIndex == colorNum)
			{
				image->colors[colorIndex * 3]	  = (char)data[(y * image->width + x)*3];
				image->colors[colorIndex * 3 + 1] = (char)data[(y * image->width + x)*3 + 1];
				image->colors[colorIndex * 3 + 2] = (char)data[(y * image->width + x)*3 + 2];
				colorNum++;
			}
		}
	}

	//define o número de cores como potência de 2 (devido aos requistos da Global Color Table)
	image->numColors = nextPower2(colorNum);

	//refine o array de cores com base no número final obtido
	copy = (char*)calloc(image->numColors*3, sizeof(char));
	memset(copy, 0, sizeof(char)*image->numColors*3);
	memcpy(copy, image->colors, sizeof(char)*colorNum*3);
	image->colors = copy;

	image->minCodeSize = numBits(image->numColors - 1);
	if (image->minCodeSize == (char)1)  //imagens binárias --> caso especial (pág. 26 do RFC)
		image->minCodeSize++;
}


//determinação da próxima potência de 2 de um dado inteiro n
int nextPower2(int n) {
	int ret = 1, nIni = n;

	if (n == 0)
		return 0;

	while (n != 0) {
		ret *= 2;
		n /= 2;
	}

	if (ret % nIni == 0)
		ret = nIni;

	return ret;
}


//número de bits necessário para representar n
char numBits(int n) {
	char nb = 0;

	if (n == 0)
		return 0;

	while (n != 0) {
		nb++;
		n /= 2;
	}

	return nb;
}


//---- Funçãoo para escrever imagem no formato GIF, versão 87a
//// COMPLETAR ESTA FUNÇÃO
void GIFEncoderWrite(imageStruct* image, char* outputFile) {

	FILE* file = fopen(outputFile, "wb");
	char trailer;

	//Escrever cabeçalho do GIF
	writeGIFHeader(image, file);

	//Escrever cabeçalho do Image Block
	// CRIAR FUNÇÃO para ESCRITA do IMAGE BLOCK HEADER!!!
	//Sugestão da assinatura do método a chamar:
	//
	writeImageBlockHeader(image, file);

	/////////////////////////////////////////
	//Escrever blocos com 256 bytes no máximo
	/////////////////////////////////////////
	//CODIFICADOR LZW AQUI !!!!
	//Sugestão de assinatura do método a chamar:
	//
	 LZWCompress(file, image);


	fprintf(file, "%c", (char)0);

	//trailer
	trailer = 0x3b;
	fprintf(file, "%c", trailer);

	fclose(file);
}


//--------------------------------------------------
//gravar cabeçalho do GIF (até global color table)
void writeGIFHeader(imageStruct* image, FILE* file) {

	int i;
	char toWrite, GCTF, colorRes, SF, sz, bgci, par;

	//Assinatura e versão (GIF87a)
	char* s = "GIF87a";
	for (i = 0; i < (int)strlen(s); i++)
		fprintf(file, "%c", s[i]);

	//Ecrã lógico (igual à da dimensão da imagem) --> primeiro o LSB e depois o MSB
	fprintf(file, "%c", (char)( image->width & 0xFF));
	fprintf(file, "%c", (char)((image->width >> 8) & 0xFF));
	fprintf(file, "%c", (char)( image->height & 0xFF));
	fprintf(file, "%c", (char)((image->height >> 8) & 0xFF));

	//GCTF, Color Res, SF, size of GCT
	GCTF = 1;
	colorRes = 7;  //número de bits por cor primária (-1)
	SF = 0;
	sz = numBits(image->numColors - 1) - 1; //-1: 0 --> 2^1, 7 --> 2^8
	toWrite = (char) (GCTF << 7 | colorRes << 4 | SF << 3 | sz);
	fprintf(file, "%c", toWrite);

	//Background color index
	bgci = 0;
	fprintf(file, "%c", bgci);

	//Pixel aspect ratio
	par = 0; // 0 --> informação sobre aspect ratio n~zo fornecida --> decoder usa valores por omissão
	fprintf(file, "%c",par);

	//Global color table
	for (i = 0; i < image->numColors * 3; i++)
		fprintf(file, "%c", image->colors[i]);
}

//------------------------------------------------------------------------------
//Meta 1

void writeImageBlockHeader(imageStruct* image, FILE* file){
	char local_color_tflag, interface_flag, sort_flag, size_lctf, reserved;
	char flags;

	// Image Separator
	fprintf(file, "%c", (char)(0x2C));

	//Image Left Position 2bytes
	fprintf(file, "%c%c", (char)(0),(char) (0));

	// Image Top Position 2bytes
	fprintf(file, "%c%c", (char)(0),(char) (0));

	// Image Width
	fprintf(file, "%c", (char)( image->width & 0xFF));
	fprintf(file, "%c", (char)((image->width >> 8) & 0xFF));

	// Image Height
	fprintf(file, "%c", (char)( image->height & 0xFF));
	fprintf(file, "%c", (char)((image->height >> 8) & 0xFF));

	// Flags
	local_color_tflag = 0;
	interface_flag = 0;
	sort_flag = 0;
	reserved = 0;
	size_lctf = 0;
	flags = (char) (local_color_tflag << 7 | interface_flag << 6 | sort_flag << 5 | reserved << 3 | size_lctf);
	fprintf(file, "%c", flags);

	// LZW Minimum Color Size
	fprintf(file, "%c", (char)(image->minCodeSize));
}


//----------------------------------------------------------------------------
//Meta Final
void LZWCompress(FILE *file, imageStruct* image){
	Dict *dict;
	Dict *element;
	int size = image->height * image->width;
	int size_dict = pow(2, (image->minCodeSize + 1));
	int clear_code, end_of_information, img_pos;
	int bloc_pos = 0;
	char caract[2];
	char buffer[4096];
	char temp[4096];
	char bloco[256];

	clear_code = pow(2,(image->minCodeSize));
	end_of_information = clear_code + 1;

	dict = (Dict *) malloc(pow(2, image->minCodeSize) * sizeof(Dict));

	for(img_pos = 0; img_pos < size; img_pos++){
		caract[0] = image->pixels[img_pos];
		caract[1] = '\n';
		
	}
}
