#include "common.h"
// font module, which utilizes the stb_truetype library by Sean Barrett / RAD Game Tools
// stb_truetype is in the public domain

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#if 1
Bitmap TTF_LoadGlyph(MemoryStack *ms, const FileInfo *ttf_file, int code_point) {
	Bitmap bm;
	byte *src, *base;
	u32 *dst;
	stbtt_fontinfo font;
	s32 w, h, ascent, descent, line_gap, baseline;
   s32 c_x0, c_y0, c_x1, c_y1;
   r32 scale;

	Assert(stbtt_InitFont(&font, (const byte *)ttf_file->data, 0));
   scale = stbtt_ScaleForPixelHeight(&font, 20.0f);
	stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
   baseline = (s32)(ascent*scale);

   stbtt_GetCodepointBitmapBox(&font, code_point, scale, scale, &c_x0, &c_y0, &c_x1, &c_y1);

   base = src = stbtt_GetCodepointBitmap(&font, 0, scale, code_point, &w, &h, 0, 0);
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
#else 

Bitmap TTF_LoadGlyph(MemoryStack *ms, const FileInfo *ttf_file, int code_point) {
	stbtt_fontinfo font;
   r32 scale;
   s32 ascent, descent, line_gap;
   Bitmap bm;
   byte *base;

	Assert(stbtt_InitFont(&font, (const byte *)ttf_file->data, 0));
   scale = stbtt_ScaleForPixelHeight(&font, 20.0f);
   stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
   ascent *= scale;
   descent *= scale;

   /* get bounding box for character (may be offset to account for chars that dip above or below the line */
   s32 c_x0, c_y0, c_x1, c_y1;
   stbtt_GetCodepointBitmapBox(&font, code_point, scale, scale, &c_x0, &c_y0, &c_x1, &c_y1);

   /* how wide is this character */
   s32 ax;
   stbtt_GetCodepointHMetrics(&font, code_point, &ax, 0);
	bm = MakeBitmap(ms, ax, c_y1-c_y0);

   /* compute y (different characters have different heights */
   s32 y = ascent + c_y1;
   /* render character (stride and offset is important here) */
   s32 byte_offset = y * (c_x1-c_x0);
   stbtt_MakeCodepointBitmap(&font, bm.data + byte_offset, c_x1 - c_x0, c_y1 - c_y0, 0, scale, scale, code_point);

   //x += ax * scale;

   ///* add kerning */
   //int kern;
   //kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
   //x += kern * scale;

   return bm;
}
#endif

#if 0
/* load font file */
long size;
unsigned char* fontBuffer;

FILE* fontFile = fopen("C:/Windows/Fonts/tahoma.ttf", "rb");
fseek(fontFile, 0, SEEK_END);
size = ftell(fontFile); /* how long is the file ? */
fseek(fontFile, 0, SEEK_SET); /* reset */

fontBuffer = malloc(size);

fread(fontBuffer, size, 1, fontFile);
fclose(fontFile);

/* prepare font */
stbtt_fontinfo info;
if (!stbtt_InitFont(&info, fontBuffer, 0))
{
   printf("failed\n");
}

int b_w = 512; /* bitmap width */
int b_h = 128; /* bitmap height */
int l_h = 64; /* line height */

              /* create a bitmap for the phrase */
unsigned char* bitmap = malloc(b_w * b_h);

/* calculate font scaling */
float scale = stbtt_ScaleForPixelHeight(&info, l_h);

char* word = "how are you?";

int x = 0;

int ascent, descent, lineGap;
stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

ascent *= scale;
descent *= scale;

int i;
for (i = 0; i < strlen(word); ++i)
{
   /* get bounding box for character (may be offset to account for chars that dip above or below the line */
   int c_x1, c_y1, c_x2, c_y2;
   stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

   /* compute y (different characters have different heights */
   int y = ascent + c_y1;

   /* render character (stride and offset is important here) */
   int byteOffset = x + (y  * b_w);
   stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

   /* how wide is this character */
   int ax;
   stbtt_GetCodepointHMetrics(&info, word[i], &ax, 0);
   x += ax * scale;

   /* add kerning */
   int kern;
   kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
   x += kern * scale;
}

/* save out a 1 channel image */
stbi_write_png("out.png", b_w, b_h, 1, bitmap, b_w);

free(fontBuffer);
free(bitmap);

return 0;
#endif


