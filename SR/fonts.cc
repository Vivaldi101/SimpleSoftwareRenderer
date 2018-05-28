#include "common.h"
// font module, which utilizes the stb_truetype library by Sean Barrett / RAD Game Tools
// stb_truetype is in the public domain

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#if 0
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

Bitmap TTF_LoadString(MemoryStack *ms, const FileInfo *ttf_file, const char *str) {
	Bitmap bm;
	stbtt_fontinfo font;
   int ascent, descent, line_gap;
   r32 scale;
   byte *src;
   u32 *dst;
   int w = 300, h = 60, x = 0;
   byte tmp_bmp_buffer[512*512] = {};

	Assert(stbtt_InitFont(&font, (const byte *)ttf_file->data, 0));
	bm = MakeBitmap(ms, w, h);

   /* calculate font scaling */
   scale = stbtt_ScaleForPixelHeight(&font, h - 15);

   stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);

   ascent *= scale;
   descent *= scale;

   for (int i = 0; i < strlen(str); ++i) {
      /* get bounding box for character (may be offset to account for chars that dip above or below the line */
      int c_x1, c_y1, c_x2, c_y2;
      stbtt_GetCodepointBitmapBox(&font, str[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

      /* compute y (different characters have different heights */
      int y = ascent + c_y1;

      /* render character (stride and offset is important here) */
      int byte_offset = x + (y  * w);
      stbtt_MakeCodepointBitmap(&font, tmp_bmp_buffer + byte_offset, c_x2 - c_x1, c_y2 - c_y1, w, scale, scale, str[i]);

      /* how wide is this character */
      int ax;
      stbtt_GetCodepointHMetrics(&font, str[i], &ax, 0);
      x += ax * scale;

      /* add kerning */
      int kern;
      kern = stbtt_GetCodepointKernAdvance(&font, str[i], str[i+1]);
      x += kern * scale;
   }

	src = tmp_bmp_buffer + (w * (h - 1));
	dst = (u32 *)bm.data;

	for (int i = 0; i < h; i++) {
		u8 *src8 = src;
		for (int j = 0; j < w; j++) {
			u8 alpha = *src8++;
			*dst++ = ((alpha << 24)|(alpha << 16)|(alpha << 8)|(alpha << 0));
		}
		src -= w;
	}

   return bm;
}
#endif



