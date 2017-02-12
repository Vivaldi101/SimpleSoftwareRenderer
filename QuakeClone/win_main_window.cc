#include "win_renderer.h"
#include "renderer_local.h"
#define	WINDOW_CLASS_NAME "QC"


#define	WINDOW_STYLE (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE)
b32 Vid_CreateWindow(Renderer *ren, int width, int height, void *wndproc, void *hinstance) {
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	VidSystem vid_sys;
	memset(&vid_sys, 0, sizeof(vid_sys));

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

	vid_sys.win_handles.window = CreateWindowEx(ex_style,
												WINDOW_CLASS_NAME,
										  		"QC Window",
										  		style_bits,
										  		x, y, w, h,
										  		0,
										  		0,
										  		(HINSTANCE)hinstance,
										  		0);
	vid_sys.width = width;
	vid_sys.height = height;

	if (!vid_sys.win_handles.window) {
		return false;
	}
	
	ShowWindow(vid_sys.win_handles.window, SW_SHOWNORMAL);
	UpdateWindow(vid_sys.win_handles.window);
	SetForegroundWindow(vid_sys.win_handles.window);
	SetFocus(vid_sys.win_handles.window);

	Sys_Print("Main window created!\n");

	memcpy(&ren->vid_sys, &vid_sys, sizeof(vid_sys));

	return true;

	// let the sound and input subsystems know about the new window
}

