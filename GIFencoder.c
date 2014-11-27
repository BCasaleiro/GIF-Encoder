#include "GIFencoder.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	printf("ahahahha");
	writeImageBlockHeader(image, file);

	/////////////////////////////////////////
	//Escrever blocos com 256 bytes no máximo
	/////////////////////////////////////////
	//CODIFICADOR LZW AQUI !!!!
	//Sugestão de assinatura do método a chamar:
	//
	printf("ola2");
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

	printf("ola");

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
	int size_dict = pow(2, (image->minCodeSize + 1)); 					//Tamanho do dicionario
	int clear_code, end_of_information, img_pos, i, remaining_bits, nbits;
	int bloc_pos = 0;
	int dict_pos = 0;
	char caract[2];
	char buffer[4096];
	char temp[4096];
	char bloco[256];
	char prev, masc, masc1, byte, byte1, temp1, temp2;

	FILE* fp = fopen("out.txt", "w");

	clear_code = pow(2,(image->minCodeSize));
	end_of_information = clear_code + 1;

	if( (dict = init_dict(size_dict, &dict_pos, image)) == NULL){
		perror("While creating dictionary!");
		return;
	}

	for(img_pos = 0; img_pos < size; img_pos++){
		caract[0] = image->pixels[img_pos];
		caract[1] = '\n';

		strcpy(temp,buffer);        //temp = buffer + caract
		strcat(temp,caract);

		if (bloc_pos == 255){
			//escreve bloco no ficheiro e volta a meter posi�ao = 0
			for (i=0;i<256;i++){
				fprintf(file, "%c", (char)( bloco[i] ));
				fprintf(fp, "%d - %d \n", i, bloco[i]);
				printf("Bloco[%i] -> %d\n",bloc_pos,bloco[i]);
			}
			//exit (0);
			img_pos--;
			continue;
			bloc_pos = 0;
		}

		if (bloc_pos == 0){
			// existe informa�ao para o bloco e estamos na posi�ao 0
			bloco[bloc_pos] = 255;
			bloc_pos++;
		}

		if (bloc_pos == 1 && img_pos == 0){
			// Se é o primeiro bloco escreve na posi�ao bloco[1] o clearcode
			bloco[bloc_pos] = (char)(clear_code);
			nbits = numBits(clear_code);
			remaining_bits = 8 - nbits;
			bloc_pos++;
		}

		if (img_pos == size-1){
			// Se é o ultimo elemento da imagem
			//nbits = numBits(buffer);

			bloco[bloc_pos]  = (char) end_of_information;
		}

		if ((element = search_element(dict,temp)) != NULL){
			// Encontra
			strcat(buffer,caract);       							// se encontra, concatena

		} else {
			//Procura o buffer

			if(dict_pos == size_dict - 1){
				size_dict = size_dict * 2;
				if(size_dict < 4096){
					double_space(dict, size_dict);
				} else if(size_dict > 4096){
					printf("Reset dictionary");
					dict_pos = 0;
					dict = reset_dict(dict, pow(2, (image->minCodeSize + 1)), &dict_pos, image );
					size_dict = pow(2, (image->minCodeSize + 1));
				}
				printf("duplicou");
			}

			if(	(element = search_element(dict, buffer)) == NULL){
				perror("while searching for buffer");
				return;
			}

			if(remaining_bits == 8){
				bloc_pos--;
				remaining_bits = 0;
			}

			if (remaining_bits == 0){

				//  Não existem bits do byte anterior para serem escritos
				bloco[bloc_pos]  = element->index;
				remaining_bits = 8-image->minCodeSize;
				bloc_pos++;
				insert_element(dict, temp);
				dict_pos++;
				strcpy(buffer,caract);

			} else if (remaining_bits > 0){
				// Se existem bits restantes
				if (bloc_pos == 1){                       			// se for na posição 1 do bloco
					prev = bloco[255];              				// vai buscar o ultimo elemento do bloco anterior para previous
				} else {                                    		// caso contrario
					prev = bloco[(bloc_pos - 1)];      				// vai buscar o elemento anterior
				}
					temp1 = bloco[bloc_pos] >> remaining_bits;
					temp2 = bloco[bloc_pos] << (8-remaining_bits);  // cria variavel temporaria com os bits na posição certa para o indice anterior

																	// de seguida utilizar mascara para escolher os necessarios
					//byte = prev;
					masc =  (char)11111111;        					// mascara para retirar os bits que precisamos preencher
					masc1 = (char)11111111;

					masc = masc >> remaining_bits;     				// mascara para o byte anterior (mete a 1 o que precisamos dele)
					masc1 = masc1 << (8 - remaining_bits);  		// mascara para o byte actual (mete a 1 o que precisamos dele)

					byte = prev & masc;      						// liga os necessarios do anterior
					byte1 = temp2 | masc1;       					// liga os necessarios do temp3

					byte = byte | byte1;

					bloco[bloc_pos-1] = byte;
					bloco[bloc_pos]= temp1;
					bloc_pos++;

					insert_element(dict, temp);
					dict_pos++;
					strcpy(buffer,caract);
				}
			}
		}
		fclose(fp);
}
//----------------------------------------------------------------------------
//Funções Auxiliares

Dict* init_dict(int size_dict, int* dict_pos, imageStruct* image){
	Dict *dict;
	int i;
	char *nkey;

	if((dict = (Dict *) malloc(size_dict * sizeof(Dict))) != NULL){

		for(i = 0; i < image->numColors; i++)
		{
			dict_pos++;
			dict[i].index = i + 1;
			nkey = (char)i;
			strcpy(dict[i].key, nkey);

	}

	}
	return dict;
}

Dict* reset_dict(Dict* dict, int new_size, int *dict_pos, imageStruct* image){
	Dict* aux;

	free(dict);
	aux = init_dict(new_size, dict_pos, image);

	return aux;
}

void insert_element(Dict *dict, char* key){
	int i = 0;

	while(dict[i].key != NULL){
		i++;
	}

	dict[i].index = dict[i - 1].index + 1;
	dict[i].key = key;
}

Dict* double_space(Dict *dict, int size_dict){
	Dict* aux;

	aux = (Dict*) realloc(dict, size_dict * sizeof(Dict));

	return aux;
}

Dict* search_element(Dict dict[], char* key){
	int i = 0;

	while (dict[i++].key != NULL) {
		if( strcmp(dict[i].key, key) == 0 ){
			return &(dict[i]);
		}
	}

	return NULL;
}
