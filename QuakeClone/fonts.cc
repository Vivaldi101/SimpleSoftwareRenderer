#include "common.h"
// font module, which utilizes the stb_truetype library by Sean Barrett / RAD Game Tools
// stb_truetype is in the public domain

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

Bitmap TTF_Init(MemoryStack *ms, const FileInfo *ttf_file, int code_point) {
	Bitmap bm;
	byte *src, *base;
	u32 *dst;

	stbtt_fontinfo font;
	int w, h;

	stbtt_InitFont(&font, (const byte *)ttf_file->data, stbtt_GetFontOffsetForIndex((const byte *)ttf_file->data, 0));
	base = src = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, 25.0f), code_point, &w, &h, 0, 0);
	bm = MakeBitmap(ms, w, h);
	src = src + (w * (h - 1));
	dst = (u32 *)bm.data;

	for (int i = 0; i < h; ++i) {
		u8 *src_8 = src;
		for (int j = 0; j < w; ++j) {
			u8 alpha = *src_8++;
			*dst++ = ((alpha << 24)|(alpha << 16)|(alpha << 8)|(alpha << 0));
		}
		src -= w;
	}

	stbtt_FreeBitmap(base, 0);

	return bm;
}

