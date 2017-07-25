#include "win_r.h"
#include "renderer.h"
#define	WINDOW_CLASS_NAME "QC"


#define	FULL_SCREEN (WS_POPUP|WS_VISIBLE|WS_SYSMENU)
#define	WINDOWED ()
b32 InitWindow(RenderTarget *rt, void *wndproc, void *hinstance) {
	WNDCLASS wc = {};

	//RECT rect;
	//int	x, y, w, h;

	int	style_bits = FULL_SCREEN;
	int ex_style_bits = WS_EX_TOPMOST;

	HDC hdc = GetDC(GetDesktopWindow());
#ifdef PLATFORM_FULLSCREEN
	int width = GetDeviceCaps(hdc, HORZRES);
	int height = GetDeviceCaps(hdc, VERTRES);
#else
	int width = GetDeviceCaps(hdc, HORZRES) / 4;
	int height = GetDeviceCaps(hdc, VERTRES) / 4;
#endif
	ReleaseDC(GetDesktopWindow(), hdc);

    wc.lpfnWndProc   = (WNDPROC)wndproc;
    wc.hInstance     = (HINSTANCE)hinstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc)) {
		return false;
	}

	//rect.left = 0;
	//rect.top = 0;
	//rect.right  = width;
	//rect.bottom = height;

	//AdjustWindowRect(&rect, style_bits, FALSE);

	//w = rect.right - rect.left;
	//h = rect.bottom - rect.top;
	//x = 0;
	//y = 0;

	rt->win_handles.window = CreateWindowEx(ex_style_bits,
											WINDOW_CLASS_NAME,
										  	"Engine",
										  	style_bits,
										  	0, 0, width, height,
										  	0,
										  	0,
										  	(HINSTANCE)hinstance,
										  	0);
	rt->width = width;
	rt->height = height;

	if (!rt->win_handles.window) {
		return false;
	}
	
	ShowWindow(rt->win_handles.window, SW_SHOWNORMAL);
	UpdateWindow(rt->win_handles.window);
	SetForegroundWindow(rt->win_handles.window);
	SetFocus(rt->win_handles.window);

	Sys_Print("Main window created!\n");

	return true;
}

