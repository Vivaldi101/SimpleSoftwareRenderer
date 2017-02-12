#include "renderer_local.h"
#include "win_renderer.h"

static HGDIOBJ	s_global_previously_selected_GDI_obj;
struct DibData {
	/*
	**	NEVER put any members before header!
	*/
	BITMAPINFOHEADER	header;
	RGBQUAD				colors[256];	// FIXME: move rgb colors someplace
};

b32 DIB_Init(VidSystem *vid_sys) {

	DibData dib;
	BITMAPINFO *win_dib_info = (BITMAPINFO *)&dib;

	memset(&dib, 0, sizeof(dib));

#if 0
	// testing the pixel format... will be removed maybe
	HMODULE ddraw_library = LoadLibrary("ddraw.dll");
	if (!ddraw_library) {
		Sys_Quit();
	}

	#define DIRECT_DRAW_CREATE(name) HRESULT WINAPI name(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter)
	typedef DIRECT_DRAW_CREATE(DDrawCreate);
	DDrawCreate* ddraw_fn_ptr = (DDrawCreate*)GetProcAddress(ddraw_library, "DirectDrawCreateEx");

	IDirectDrawSurface7 *primary_surface;
	LPDIRECTDRAW7 lpdd;
	DDSURFACEDESC2 ddsd;						

    // create IDirectDraw interface 7.0 object and test for error
	if (FAILED(ddraw_fn_ptr(NULL, (LPVOID *)&lpdd, IID_IDirectDraw7, NULL))) {
		Sys_Print("Failed to init ddraw object");
	}

	// set cooperation level to windowed mode 
	if (FAILED(lpdd->SetCooperativeLevel(0, DDSCL_NORMAL))) {
		Sys_Print("Failed set the co-op level for ddraw");
	}

	#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

    // Create the primary surface
	DDRAW_INIT_STRUCT(ddsd);

	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    lpdd->CreateSurface(&ddsd, &primary_surface, 0);

    // get the pixel format of the primary surface
    DDPIXELFORMAT ddpf; // used to get pixel format

    // initialize structure
    DDRAW_INIT_STRUCT(ddpf);

    // query the format from primary surface
    primary_surface->GetPixelFormat(&ddpf);

	int rgb_mode = ddpf.dwRGBBitCount;

	if (ddraw_library) {
		FreeLibrary(ddraw_library);
	}
#endif

	/*
	** grab a DC
	*/
	if (!vid_sys->win_handles.dc) {
		if (!(vid_sys->win_handles.dc = GetDC(vid_sys->win_handles.window))) {
			Sys_Print("\nCouldn't get dc for window\n");
			return false;
		}
	}

	//printf("WinGDI: global_soft_renderer.palettized =%d\n",global_soft_renderer.palettized );

	vid_sys->bpp = 4;

	/*
	** fill in the BITMAPINFO struct
	*/
	win_dib_info->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	win_dib_info->bmiHeader.biWidth         = vid_sys->width;		// get the buffer
	win_dib_info->bmiHeader.biHeight        = -vid_sys->height;		// top down buffer
	win_dib_info->bmiHeader.biPlanes        = 1;
	win_dib_info->bmiHeader.biBitCount      = (WORD)(vid_sys->bpp * 8);
	win_dib_info->bmiHeader.biCompression   = BI_RGB;
	win_dib_info->bmiHeader.biSizeImage     = 0;
	win_dib_info->bmiHeader.biXPelsPerMeter = 0;
	win_dib_info->bmiHeader.biYPelsPerMeter = 0;
	win_dib_info->bmiHeader.biClrUsed       = /*256*/ 0;
	win_dib_info->bmiHeader.biClrImportant  = /*256*/ 0;

#if 0
	/*
	** fill in the palette
	*/
	for (int i = 0; i < 256; ++i) {
		//dib.colors[i].rgbRed   = (global_8to24able[i] >> 0)  & 0xff;
		//dib.colors[i].rgbGreen = (global_8to24able[i] >> 8)  & 0xff;
		//dib.colors[i].rgbBlue  = (global_8to24able[i] >> 16) & 0xff;

		// placeholder
		dib.colors[i].rgbRed   = (byte)i;
		dib.colors[i].rgbGreen = (byte)i;
		dib.colors[i].rgbBlue  = (byte)i;
	}
#endif

	/*
	** create the DIB section
	*/
	vid_sys->win_handles.dib_section = CreateDIBSection(vid_sys->win_handles.dc,
														win_dib_info,
											 			DIB_RGB_COLORS,
														(void **)&vid_sys->buffer,
											 			0,
											 			0);

	if (!vid_sys->win_handles.dib_section) {
		//ri.Con_Printf( PRINT_ALL, "DIB_Init() - CreateDIBSection failed\n" );
		Sys_Print("\nCouldn't create the dib section\n");
		return false;
	}

	if (win_dib_info->bmiHeader.biHeight > 0) {
		// bottom up
		//vid_sys->buffer	= vid_sys->buffer + (global_vid_sys.height - 1 ) * global_vid_sys.width;
		vid_sys->pitch	= -vid_sys->width * (dib.header.biBitCount / 8);
		Sys_Print("\nWinGDI, framebuffer is bottomup.\n");
    } else {
		// top down
		//vid_sys->buffer	= global_soft_renderer.dib_data;
		vid_sys->pitch	= vid_sys->width * (dib.header.biBitCount / 8);
		Sys_Print("\nWinGDI, framebuffer is top down.\n");
    }

	//printf("CreateDIBSection OK: w=%d, h=%d, buffer=%x\n",vid.width,vid.height,vid.buffer);

	/*
	** clear the DIB memory buffer
	*/

	memset(vid_sys->buffer, 0xff, vid_sys->pitch * vid_sys->height);

	if (!(vid_sys->win_handles.hdc_dib_section = CreateCompatibleDC(vid_sys->win_handles.dc))) {
		//ri.Con_Printf( PRINT_ALL, "DIB_Init() - CreateCompatibleDC failed\n" );
		Sys_Print("\nDIB_Init() - CreateCompatibleDC failed\n");
		return false;
		//goto fail;
	}
	if (!(s_global_previously_selected_GDI_obj = SelectObject(vid_sys->win_handles.hdc_dib_section, vid_sys->win_handles.dib_section))) {
		Sys_Print("\nDIB_Init() - SelectObject failed\n");
		return false;
		//goto fail;
	}

	return true;
}
