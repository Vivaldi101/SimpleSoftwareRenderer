#include "renderer.h"
#include "win_r.h"

// FIXME: testing stuff
#if 1
// sample a 24-bit RGB value to one of the colours on the existing 8-bit palette
static int Convert24To8(const byte palette[256*3], const int rgb[3]) {
	int best_index = -1;
	int best_dist = INT32_MAX;

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

b32 InitDIB(VidSystem *vid_sys) {

	DibData dib;
	BITMAPINFO *win_dib_info = (BITMAPINFO *)&dib;

	memset(&dib, 0, sizeof(dib));

	if (!vid_sys->win_handles.hdc) {
		if (!(vid_sys->win_handles.hdc = GetDC(vid_sys->win_handles.window))) {
			Sys_Print("\nCouldn't get dc for window\n");
			return false;
		}
	}

	vid_sys->bpp = BYTES_PER_PIXEL;

	win_dib_info->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	win_dib_info->bmiHeader.biWidth         = vid_sys->width;		
	win_dib_info->bmiHeader.biHeight        = vid_sys->height; 
	win_dib_info->bmiHeader.biPlanes        = 1;
	win_dib_info->bmiHeader.biBitCount      = (WORD)(vid_sys->bpp * 8);
	win_dib_info->bmiHeader.biCompression   = BI_RGB;
	win_dib_info->bmiHeader.biSizeImage     = 0;
	win_dib_info->bmiHeader.biXPelsPerMeter = 0;
	win_dib_info->bmiHeader.biYPelsPerMeter = 0;
	win_dib_info->bmiHeader.biClrUsed       = /*256*/ 0;
	win_dib_info->bmiHeader.biClrImportant  = /*256*/ 0;

	// FIXME: this is only here for testing!!!!!!!!
	if (BYTES_PER_PIXEL == 1) {
		// FIXME: file io elsewhere
		FILE *fp;
		fopen_s(&fp, "palette.lmp", "r");
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		Assert(size == 256 * 3);
		fseek(fp, 0, SEEK_SET);

		if (!fp) {
			Sys_Print("\nCouldn't open palette.lmp file\n");
			Sys_Quit();
		}

		byte palette[256*3];
		byte *ptr = palette;
		fread_s(palette, size, 1, size, fp);

		//Assert(vid_sys->colormap);
		GenerateColormap(palette, vid_sys->colormap);

	#if 1
		for (int i = 0; i < 256; ++i) {
			dib.colors[i].rgbRed   = ptr[0];
			dib.colors[i].rgbGreen = ptr[1];
			dib.colors[i].rgbBlue  = ptr[2];

			ptr += 3;
		}
	#endif

		if (fp) {
			Sys_Print("\nClosing pal file\n");
			fclose(fp);
		}
	}
	vid_sys->win_handles.dib_section = CreateDIBSection(vid_sys->win_handles.hdc,
														win_dib_info,
											 			DIB_RGB_COLORS,
														(void **)&vid_sys->buffer,
											 			0,
											 			0);

	if (!vid_sys->win_handles.dib_section) {
		Sys_Print("\nCouldn't create the dib section\n");
		return false;
	}

	vid_sys->pitch = vid_sys->width * (dib.header.biBitCount / 8);

	memset(vid_sys->buffer, 0, vid_sys->pitch * vid_sys->height);

	if (!(vid_sys->win_handles.hdc_dib_section = CreateCompatibleDC(vid_sys->win_handles.hdc))) {
		Sys_Print("\nDIB_Init() - CreateCompatibleDC failed\n");
		return false;
	}
	if (!(global_previously_selected_GDI_obj = SelectObject(vid_sys->win_handles.hdc_dib_section, vid_sys->win_handles.dib_section))) {
		Sys_Print("\nDIB_Init() - SelectObject failed\n");
		return false;
	}

	return true;
}
