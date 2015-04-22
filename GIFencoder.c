#include "GIFencoder.h"
#include "BitFile.h"
#include "BitFile.c"

// convers�o de um objecto do tipo Image numa imagem indexada
imageStruct* GIFEncoder(unsigned char *data, int width, int height) {

	imageStruct* image = (imageStruct*)malloc(sizeof(imageStruct));
	image->width = width;
	image->height = height;

	//converter para imagem indexada
	RGB2Indexed(data, image);

	return image;
}



//convers�o de lista RGB para indexada: m�ximo de 256 cores
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

	//define o n�mero de cores como pot�ncia de 2 (devido aos requistos da Global Color Table)
	image->numColors = nextPower2(colorNum);

	//refine o array de cores com base no n�mero final obtido
	copy = (char*)calloc(image->numColors*3, sizeof(char));
	memset(copy, 0, sizeof(char)*image->numColors*3);
	memcpy(copy, image->colors, sizeof(char)*colorNum*3);
	image->colors = copy;

	image->minCodeSize = numBits(image->numColors - 1);
	if (image->minCodeSize == (char)1)  //imagens bin�rias --> caso especial (p�g. 26 do RFC)
		image->minCodeSize++;
}


//determina��o da pr�xima pot�ncia de 2 de um dado inteiro n
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


//n�mero de bits necess�rio para representar n
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


//---- Fun��o para escrever imagem no formato GIF, vers�o 87a
//// COMPLETAR ESTA FUN��O
void GIFEncoderWrite(imageStruct* image, char* outputFile) {

	FILE* file = fopen(outputFile, "wb");
	char trailer;

	//Escrever cabe�alho do GIF
	writeGIFHeader(image, file);

	//Escrever cabe�alho do Image Block
	// CRIAR FUN��O para ESCRITA do IMAGE BLOCK HEADER!!!
	//Sugest�o da assinatura do m�todo a chamar:
	//
	writeImageBlockHeader(image, file);

	/////////////////////////////////////////
	//Escrever blocos com 256 bytes no m�ximo
	/////////////////////////////////////////
	//CODIFICADOR LZW AQUI !!!!
	//Sugest�o de assinatura do m�todo a chamar:
	//
	LZWCompress(file, image->minCodeSize, image->pixels, image->width*image->height, image->numColors);


	fprintf(file, "%c", (char)0);

	//trailer
	trailer = 0x3b;
	fprintf(file, "%c", trailer);

	fclose(file);
}


//--------------------------------------------------
//gravar cabe�alho do GIF (at� global color table)
void writeGIFHeader(imageStruct* image, FILE* file) {

	int i;
	char toWrite, GCTF, colorRes, SF, sz, bgci, par;

	//Assinatura e vers�o (GIF87a)
	char* s = "GIF87a";
	for (i = 0; i < (int)strlen(s); i++)
		fprintf(file, "%c", s[i]);

	//Ecr� l�gico (igual � da dimens�o da imagem) --> primeiro o LSB e depois o MSB
	fprintf(file, "%c", (char)( image->width & 0xFF));
	fprintf(file, "%c", (char)((image->width >> 8) & 0xFF));
	fprintf(file, "%c", (char)( image->height & 0xFF));
	fprintf(file, "%c", (char)((image->height >> 8) & 0xFF));

	//GCTF, Color Res, SF, size of GCT
	GCTF = 1;
	colorRes = 7;  //n�mero de bits por cor prim�ria (-1)
	SF = 0;
	sz = numBits(image->numColors - 1) - 1; //-1: 0 --> 2^1, 7 --> 2^8
	toWrite = (char) (GCTF << 7 | colorRes << 4 | SF << 3 | sz);
	fprintf(file, "%c", toWrite);

	//Background color index
	bgci = 0;
	fprintf(file, "%c", bgci);

	//Pixel aspect ratio
	par = 0; // 0 --> informa��o sobre aspect ratio n�o fornecida --> decoder usa valores por omiss�o
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

//------------------------------------------------------------------------------
//Meta Final

void LZWCompress(FILE* file, int minCodeSize, char* pixels, int npixels, int ncolors){
	int imgPos = 0;													// posição na imagem
	int dictPos = 0; 												// nº de elementos no dicionário
	int dictSize = pow(2, minCodeSize + 1);								// tamanho do dicionário
	int clearCode, endOfInformation, pos;
	char* c;
	char temp[4096];
	char buffer[4096];
	Dict* dict;
	bitStream* stream = bitFile(file);

	if( (dict = initDict(dictSize)) == NULL ){ 						// criação do dicionário
		perror("While creating dictionary\nExiting...\n");
		return;
	}

	clearCode = pow(2, minCodeSize);								// clear code
	endOfInformation = pow(2, minCodeSize) + 1;						// end of information
	fillDict(dict, &dictPos, ncolors, clearCode, endOfInformation); // inicia com o alfabeto

	sprintf(buffer, "%d", pixels[imgPos++]);

	writeBits(stream, clearCode, numBits(dictPos - 1));

	for(; imgPos < npixels; imgPos++){
		c = malloc(ndigits(imgPos) * sizeof(char));
		sprintf(c, "%d", pixels[imgPos]);

		strcpy(temp, buffer);										// temp = buffer + "," + c
		strcat(temp, ",");
		strcat(temp, c);											// buffer permanece intacto

		if(dictPos == dictSize){									//sempre que necessário o espaço do dicionário é duplicado
			if(dictSize < 4096){
				dictSize = dictSize * 2;
				dict = doubleDictSpace(dict, dictSize);
			}
		}

		if(searchInDict(dict, dictPos, temp) != -1){			// se (buffer + c) existir no dicionário concatena-os e continua
			strcat(buffer, ",");
			strcat(buffer, c);
		} else {	 	 										// caso contrario insere no dicionario (buffer + c)

			pos = searchInDict(dict, dictPos, buffer);

			writeBits(stream, pos, numBits(dictPos - 1));

			if(dictPos < 4096){									// ao chegar aos 4096 elementos o dicionário é congelado
				insertInDict(dict, dictPos, temp);
				dictPos++;
			}

			strcpy(buffer, c);
		}
		free(c);												// liberta a memória ocupada por c
	}
	writeBits(stream, endOfInformation, numBits(dictPos - 1));

	//printDict(dict, dictPos);
	//printf("Size Dict:%d\tSize Pos: %d\n", dictSize, dictPos);
	free(dict);
	flush(stream);
}

Dict* initDict(int size){
	Dict* aux;

	aux = malloc(size*sizeof(Dict));

	return aux;
}

void fillDict(Dict* dict, int *dictPos, int ncolors, int clearCode, int endOfInformation){
	int i;

	for(i = 0; i < ncolors; i++){
		dict[i].index = i;
		dict[i].key = malloc(ndigits(i) * sizeof(char));
		sprintf(dict[i].key, "%d", i);
	}
	//clearCode
	dict[i].index = i;
	dict[i].key = malloc(ndigits(clearCode) * sizeof(char));
	sprintf(dict[i].key, "%d", clearCode);
	//endOfInformation
	i++;
	dict[i].index = i;
	dict[i].key = malloc(ndigits(endOfInformation) * sizeof(char));
	sprintf(dict[i].key, "%d", endOfInformation);
	i++;
	(*dictPos) = i;
}

int searchInDict(Dict* dict, int dictPos, char* key){
	int i;

<<<<<<< HEAD
	for(img_pos = 0; img_pos < size; img_pos++){
		caract[0] = image->pixels[img_pos];
		caract[1] = '\n';


=======
	for(i = 0; i < dictPos; i++){
		//printf("[Search] Dict.key: %s\tkey:%s\tcmp: %d\n", dict[i].key, key, strcmp(dict[i].key, key));
		if(strcmp(dict[i].key, key) == 0){
			return i;
		}
>>>>>>> origin/master
	}

	return -1;
}

void insertInDict(Dict *dict, int dictPos, char* key){
	//printf("[Insert]Dict Pos: %d\tindex: %d\tkey: %s\n", dictPos, dictPos +1, key);
	dict[dictPos].index = dictPos;
	dict[dictPos].key = malloc(sizeof(*key));
	strcpy(dict[dictPos].key, key);
}

Dict* doubleDictSpace(Dict *dict, int dictSize){

	return (Dict*) realloc(dict, dictSize * sizeof(Dict));
}

void printDict(Dict *dict, int dictPos){
	int i;
	FILE* fp = fopen("testDict.txt", "w");

	for(i = 0; i < dictPos; i++){
		fprintf(fp, "Entrada nº%d\tIndex: %d\tKey: %s\n", i, dict[i].index, dict[i].key);
	}

	fclose(fp);
}

int ndigits(int n){
	int n_digits = 0;

	do{
		n_digits++;
	}while((n = n / 10) != 0);

	return n_digits;
}
