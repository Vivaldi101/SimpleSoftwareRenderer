#ifndef __INO_BMP_H__
#define __INO_BMP_H__

//#include "ino_shared.h"

#ifdef INO_BMP_STATIC
#define INO_BMP_DEF static
#else
#define INO_BMP_DEF extern
#endif

// IMPORTANT!!!
// this BMP image loader handles only BMP version 2(Microsoft Windows 2.x) with bits per pixel being 24

// api usage
// void *p = file_data("test.bmp");
// int s = file_size("test.bmp");
// void *bmp_bytes = bmp_load(p, s, allocator, &w, &h);


// sanity checks
typedef unsigned char validate_u32[sizeof(u32) == 4 ? 1 : -1];
typedef unsigned char validate_u16[sizeof(u16) == 2 ? 1 : -1];

// image format independent
typedef struct 
{
	s32 width, height;                              // image dimensions in pixels
	byte *buffer,          *buffer_end;
	byte *buffer_original, *buffer_original_end;
} img_context_t;

// image format dependent
typedef struct
{
	s32 bpp, offset, bitmap_header_size;
   u32 mr, mg, mb;
} bmp_data_t;

// IMPLEMENTATION
#ifdef INO_BMP_IMPL
#pragma warning(push)
#pragma warning(disable : 4127)

static u8 bmp_get8_bits(img_context_t *ic) 
{
   if (ic->buffer < ic->buffer_end) {
      return *(ic->buffer++);
   } 
   InvalidCodePath("Buffer exhausted!");
   return 0;
} 

static u16 bmp_get16_bits_le(img_context_t *ic) 
{
   byte b = bmp_get8_bits(ic);
   return b + (bmp_get8_bits(ic) << 8);
}

static u32 bmp_get32_bits_le(img_context_t *ic) 
{
   u16 b = bmp_get16_bits_le(ic);
   return b + (bmp_get16_bits_le(ic) << 16);
}

INO_BMP_DEF void bmp_parse_file_header(img_context_t *ic, bmp_data_t *bd)
{
   if (bmp_get8_bits(ic) != 'B' || bmp_get8_bits(ic) != 'M') {
      InvalidCodePath("Corrupted bmp file!");
   }
   bmp_get32_bits_le(ic);  // discard filesize
   bmp_get16_bits_le(ic);  // discard reserved
   bmp_get16_bits_le(ic);  // discard reserved

   bd->offset = bmp_get32_bits_le(ic);
}

INO_BMP_DEF void bmp_parse_bitmap_header(img_context_t *ic, bmp_data_t *bd)
{
   int bitmap_header_size;
   bitmap_header_size = bd->bitmap_header_size = bmp_get32_bits_le(ic);
   bd->mr = (u32)0xff << 16;
   bd->mg = (u32)0xff <<  8;
   bd->mb = (u32)0xff <<  0;
   //bd->ma = (u32)0xff << 24;
   //bd->all_a = 0; 

   if (bitmap_header_size != 12 
    && bitmap_header_size != 40 
    && bitmap_header_size != 56 
    && bitmap_header_size != 108 
    && bitmap_header_size != 124) {
      InvalidCodePath("Corrupted bmp file!");
   } 
   if (bitmap_header_size == 12) {
      ic->width = bmp_get16_bits_le(ic);
      ic->height = bmp_get16_bits_le(ic);
   } else {
      ic->width = bmp_get32_bits_le(ic);
      ic->height = bmp_get32_bits_le(ic);
   }

   if (bmp_get16_bits_le(ic) != 1) {
      InvalidCodePath("Corrupted bmp file!");
   }
   bd->bpp = bmp_get16_bits_le(ic);
}

INO_BMP_DEF void bmp_parse(img_context_t *ic, bmp_data_t *bd)
{
   bmp_parse_file_header(ic, bd);
   bmp_parse_bitmap_header(ic, bd);
}

INO_BMP_DEF void bmp_set_memory_decode_context(img_context_t *ic, const void *memory, int file_size)
{
	ic->buffer = ic->buffer_original = (byte *)memory;
	ic->buffer_end = ic->buffer_original_end = (byte *)memory+file_size;
}

INO_BMP_DEF byte *bmp_load(const void *memory, int file_size, void *(allocator)(int size), int *width, int *height)
{
   byte *result;
   img_context_t ic;
   bmp_data_t bd;
   s32 scan_line_pad;      // bmp format pads scan line to 4 byte boundary
   s32 bytes_per_pixel;
   s32 img_size_in_bytes, img_width_in_bytes;
   u32 mr, mg, mb, ma;
   u32 is_top_down;

   if (file_size <= 0) {
      InvalidCodePath("Invalid bmp file size!");
   }

   bmp_set_memory_decode_context(&ic, memory, file_size);
   bmp_parse(&ic, &bd);
   if (bd.bpp != 24) {
      InvalidCodePath("Invalid bmp bpp, must be 24!");
   }

   mr = bd.mr;
   mg = bd.mg;
   mb = bd.mb;
   //ma = bd.ma;

   bytes_per_pixel = bd.bpp >> 3;

   is_top_down = (ic.height < 0);
   img_width_in_bytes = ic.width * bytes_per_pixel;
   scan_line_pad = (-img_width_in_bytes) & 3;  

   *width = ic.width;
   *height = ic.height = abs(ic.height);
   //img_size_in_bytes = ic.width * ic.height * bytes_per_pixel;    
   img_size_in_bytes = ic.width * ic.height * 4;    

   if (allocator == NULL) {
      result = (byte *)malloc(img_size_in_bytes);   
   } else {
      result = (byte *)allocator(img_size_in_bytes);   
   }
   if (result) { 
      int i, j, z = 0;
      ic.buffer = ic.buffer_original;
      ic.buffer += bd.offset;
      
      for (i = 0; i < ic.height; i++) {
         for (j = 0; j < ic.width; j++) {
            result[z+0] = bmp_get8_bits(&ic);   // blue
            result[z+1] = bmp_get8_bits(&ic);   // green
            result[z+2] = bmp_get8_bits(&ic);   // red
            result[z+3] = 255;                  // alpha
            z += 4;
         }
         ic.buffer += scan_line_pad;
      }
   } 

   return result;
}


#pragma warning(pop)
#endif	
#endif	//	__INO_BMP_H__ 
