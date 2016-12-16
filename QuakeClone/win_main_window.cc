#include "win_renderer.h"
#define	WINDOW_CLASS_NAME "QC"


#define	WINDOW_STYLE (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE)
void Vid_CreateWindow(RendererState *rs, int width, int height, void *wndproc, void *hinstance) {
	//cvar		*vid_xpos, *vid_ypos, *vid_fullscreen;

	// FIXME: make these memsets
	WNDCLASS	wc;
	memset(&wc, 0, sizeof(wc));

	VidSystem	vid_sys;
	memset(&vid_sys, 0, sizeof(vid_sys));

	RECT		r;
	int			x, y, w, h;
	int			ex_style = WS_EX_TOPMOST;
	int			style_bits = 0;

	// Register the frame class 
    wc.lpfnWndProc   = (WNDPROC)wndproc;
    wc.hInstance     = (HINSTANCE)hinstance;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc) ) {
		//ri.Sys_Error (ERR_FATAL, "Couldn't register window class");
		abort();
	}

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect(&r, style_bits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;
	//x = vid_xpos->value;
	//y = vid_ypos->value;
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
		Sys_Print("Couldn't create the main window!\n");
		abort();	// For now
	}
	
	ShowWindow(vid_sys.win_handles.window, SW_SHOWNORMAL);
	UpdateWindow(vid_sys.win_handles.window);
	SetForegroundWindow(vid_sys.win_handles.window);
	SetFocus(vid_sys.win_handles.window);

	Sys_Print("Main window created!\n");

	memcpy(&rs->vid_sys, &vid_sys, sizeof(vid_sys));
	//global_vid_sys.win_handles = s_win_handles;

	// let the sound and input subsystems know about the new window
	//ri.Vid_NewWindow (width, height);
}

