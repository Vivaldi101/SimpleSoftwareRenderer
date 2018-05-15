#include "renderer.h"
#include "win_r.h"

#if 0
// sample a 24-bit RGB value to one of the colours on the existing 8-bit palette
static int Convert24To8(const byte palette[256*3], const int rgb[3]) {
	int best_index = -1;
	int best_dist = SINT32_MAX;

	for (int i = 0; i < 256; ++i) {
		int dist = 0;

		for (int j = 0; j < 3; ++j) {
			// note that we could use RGB luminosity bias for greater accuracy
			int d = ABS(rgb[j] - palette[i*3+j]);
			dist += d * d;
		}

		if (dist < best_dist) {
			best_index = i;
			best_dist = dist;
		}
	}

	Assert(best_index >= 0);
	return best_index;
}

static void GenerateColormap(const byte palette[256*3], byte out_colormap[256*64]) {
	int num_fullbrights = 24; 

	for (int x = 0; x < 256; ++x) {
		for (int y = 0; y < 64; ++y) {
			if (x < 256 - num_fullbrights) {
				int rgb[3];

				for (int i = 0; i < 3; ++i) {
					rgb[i] = (palette[x*3+i] * y) / 32; 
					if (rgb[i] > 255) {
						rgb[i] = 255;
					}
				}
				out_colormap[y*256+x] = (byte)Convert24To8(palette, rgb);

			} else {
				// this colour is a fullbright, just keep the original colour
				out_colormap[y*256+x] = (byte)x;
			}
		}
	}
}
#endif


static HGDIOBJ	global_previously_selected_GDI_obj;
struct DibData {
	BITMAPINFOHEADER	header;
	RGBQUAD				colors[256];	
};

b32 InitDIB(RenderTarget *rt) {

	DibData dib;
	BITMAPINFO *win_dib_info = (BITMAPINFO *)&dib;

	memset(&dib, 0, sizeof(dib));

	if (!rt->win_handles.hdc) {
		if (!(rt->win_handles.hdc = GetDC(rt->win_handles.window))) {
			Sys_Print("\nCouldn't get dc for window\n");
			return false;
		}
	}

	rt->bpp = BYTES_PER_PIXEL;

	win_dib_info->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	win_dib_info->bmiHeader.biWidth         = rt->width;		
	win_dib_info->bmiHeader.biHeight        = rt->height;    // NOTE: y-up in screen space
	win_dib_info->bmiHeader.biPlanes        = 1;
	win_dib_info->bmiHeader.biBitCount      = (WORD)(rt->bpp * 8);
	win_dib_info->bmiHeader.biCompression   = BI_RGB;
	win_dib_info->bmiHeader.biSizeImage     = 0;
	win_dib_info->bmiHeader.biXPelsPerMeter = 0;
	win_dib_info->bmiHeader.biYPelsPerMeter = 0;
	win_dib_info->bmiHeader.biClrUsed       = 0;
	win_dib_info->bmiHeader.biClrImportant  = 0;

	rt->win_handles.dib_section = CreateDIBSection(rt->win_handles.hdc,
														win_dib_info,
											 			DIB_RGB_COLORS,
														(void **)&rt->buffer,
											 			0,
											 			0);

	if (!rt->win_handles.dib_section) {
		Sys_Print("\nCouldn't create the dib section\n");
		return false;
	}

	rt->pitch = rt->width * BYTES_PER_PIXEL;

	memset(rt->buffer, 0, rt->pitch * rt->height);

	if (!(rt->win_handles.hdc_dib_section = CreateCompatibleDC(rt->win_handles.hdc))) {
		Sys_Print("\nDIB_Init() - CreateCompatibleDC failed\n");
		return false;
	}
	if (!(global_previously_selected_GDI_obj = SelectObject(rt->win_handles.hdc_dib_section, rt->win_handles.dib_section))) {
		Sys_Print("\nDIB_Init() - SelectObject failed\n");
		return false;
	}

	return true;
}
