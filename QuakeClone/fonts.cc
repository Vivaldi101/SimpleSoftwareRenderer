#include "common.h"
// font module, which utilizes the stb_truetype library by Sean Barrett / RAD Game Tools
// stb_truetype is in the public domain

#define STB_TRUETYPE_IMPLEMENTATION
//#define STBTT_STATIC
#include "stb_truetype.h"

void TTF_Init(FileIO *fio) {
	FileInfo ttf_file;
	stbtt_fontinfo font;
	byte *bitmap;
	int w, h, i, j;

	ttf_file = fio->read_file("C:/Windows/Fonts/arial.ttf");
	stbtt_InitFont(&font, (const byte *)ttf_file.data, stbtt_GetFontOffsetForIndex((const byte *)ttf_file.data, 0));
	bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, 50.0f), 'N', &w, &h, 0, 0);

	stbtt_FreeBitmap(bitmap, 0);
	fio->free_file(&ttf_file);
}

