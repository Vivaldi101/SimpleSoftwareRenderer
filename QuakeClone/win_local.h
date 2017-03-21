#ifndef WIN_LOCAL_H
#define WIN_LOCAL_H

#include "shared.h"

// console system
extern void	Sys_CreateConsole(HINSTANCE hinstance);
extern void	Sys_DestroyConsole();
extern void Sys_Print(const char *msg);
extern void Sys_Sleep(DWORD ms);
extern void Con_AppendText(const char *msg);

// com system
extern void Sys_Init();
extern void Sys_Quit();

// input system
extern void In_Activate(b32 active);
extern void In_MouseEvent(int mstate);

extern void	In_Init();
extern void	In_Shutdown();

extern void	In_DeactivateMouse();

extern void	In_Activate(b32 active);
extern void	In_Frame();

// window creation
extern b32 Vid_CreateWindow(struct Renderer *ren, int width, int height, void *wndproc, void *hinstance);
extern void Sys_ToggleFullscreen(HWND window);

// events
extern void Sys_QueEvent(int time, enum SysEventType ev_type, int value, int value2, int data_len, void *data);

LRESULT WINAPI MainWndProc(HWND window, UINT umsg, WPARAM wparam, LPARAM lparam);
struct WinVars {
	void *			wndproc;
	HINSTANCE		reflib_library;		// handle to refresh DLL 
	b32				reflib_active;

	HWND			hwnd;
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
