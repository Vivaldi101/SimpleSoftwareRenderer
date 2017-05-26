#ifndef WIN_LOCAL_H
#define WIN_LOCAL_H

#include "shared.h"


// System
extern void Sys_Init();
extern void Sys_Quit();
extern void Sys_Sleep(DWORD ms);
extern void Sys_Print(const char *msg, ...);

extern b32 InitWindow(struct RenderTarget *rt, void *wndproc, void *hinstance);
//extern b32 InitWindow(struct RenderTarget *rt, int width, int height, void *wndproc, void *hinstance);
extern void Sys_ToggleFullscreen(HWND window);

// Events
extern void Sys_QueEvent(int time, enum SysEventType ev_type, int value, int value2, int data_len, void *data);

LRESULT WINAPI MainWndProc(HWND window, UINT umsg, WPARAM wparam, LPARAM lparam);
struct WinVars {
	//void *			wndproc;
	HINSTANCE		reflib_library;		// handle to refresh DLL 
	b32				reflib_active;

	//HWND			hwnd;
	//HINSTANCE		hinstance;
	b32				active_app;
	b32				is_minimized;
	OSVERSIONINFO	os_version;

	// when we get a windows message, we store the time off so keyboard processing
	// can know the exact time of an event
	u32				sys_msg_time;
};
extern WinVars global_win_vars;
#endif	// header guard
