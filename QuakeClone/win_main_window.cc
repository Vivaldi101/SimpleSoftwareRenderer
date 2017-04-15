#include "win_r.h"
#include "renderer.h"
#define	WINDOW_CLASS_NAME "QC"


#define	WINDOW_STYLE (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE)
b32 InitWindow(VidSystem *vid_sys, int width, int height, void *wndproc, void *hinstance) {
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	RECT rect;
	int	x, y, w, h;
	int	ex_style = WS_EX_TOPMOST;
	int	style_bits = 0;

	// Register the frame class 
    wc.lpfnWndProc   = (WNDPROC)wndproc;
    wc.hInstance     = (HINSTANCE)hinstance;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc) ) {
		return false;
	}

	rect.left = 0;
	rect.top = 0;
	rect.right  = width;
	rect.bottom = height;

	AdjustWindowRect(&rect, style_bits, FALSE);

	w = rect.right - rect.left;
	h = rect.bottom - rect.top;
	x = 0;
	y = 0;

	vid_sys->win_handles.window = CreateWindowEx(ex_style,
												WINDOW_CLASS_NAME,
										  		"Test",
										  		style_bits,
										  		x, y, w, h,
										  		0,
										  		0,
										  		(HINSTANCE)hinstance,
										  		0);
	vid_sys->width = width;
	vid_sys->height = height;

	if (!vid_sys->win_handles.window) {
		return false;
	}
	
	ShowWindow(vid_sys->win_handles.window, SW_SHOWNORMAL);
	UpdateWindow(vid_sys->win_handles.window);
	SetForegroundWindow(vid_sys->win_handles.window);
	SetFocus(vid_sys->win_handles.window);

	Sys_Print("Main window created!\n");

	return true;
}

