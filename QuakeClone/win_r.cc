#include "renderer.h"
#include "win_r.h"

static HGDIOBJ	s_global_previously_selected_GDI_obj;
struct DibData {
	BITMAPINFOHEADER	header;
	RGBQUAD				colors[256];	
};

b32 DIB_Init(VidSystem *vid_sys) {

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

	if (BYTES_PER_PIXEL == 1) {
		// FIXME: this is only here for testing!!!!!!!!
		// FIXME: do proper integrity checking!!!
		FILE *fp;
		fopen_s(&fp, "palette.lmp", "r");
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		Assert(size == 256 * 3);
		fseek(fp, 0, SEEK_SET);

		if (!fp) {
			Sys_Print("\nCouldn't open pal file\n");
			Sys_Quit();
		}

		// FIXME: remove calloc and put on temp memory stack
		byte *rgb_data = (byte *)calloc(size, 1);
		byte *ptr = (byte *)rgb_data;
		fread_s(rgb_data, size, 1, size, fp);

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
		// FIXME: remove calloc and put on temp memory stack
		if(rgb_data) {
			free(rgb_data);
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
	if (!(s_global_previously_selected_GDI_obj = SelectObject(vid_sys->win_handles.hdc_dib_section, vid_sys->win_handles.dib_section))) {
		Sys_Print("\nDIB_Init() - SelectObject failed\n");
		return false;
	}

	return true;
}
