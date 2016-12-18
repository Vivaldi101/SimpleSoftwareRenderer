#ifndef QC_WIN_LOCAL_H
#define QC_WIN_LOCAL_H

#ifndef INITGUID
#define INITGUID
#endif

#include <Windows.h>
#include "shared.h"


// Console system
extern void	Sys_CreateConsole();
extern void	Sys_DestroyConsole();
extern void Sys_Print(const char *msg);
extern void Sys_Sleep(DWORD ms);
extern void Conbuf_AppendText(const char *msg);

// Com System
extern void Sys_Init();
extern void Sys_Quit();

// Input system
extern void In_Activate(b32 active);
extern void In_MouseEvent(int mstate);

extern void	In_Init();
extern void	In_Shutdown();


extern void	In_DeactivateMouse();

extern void	In_Activate(b32 active);
extern void	In_Frame();

// Windows
extern void Vid_CreateWindow(struct RendererState *rs, int width, int height, void *wndproc, void *hinstance);
extern void Sys_ToggleFullscreen(HWND window);

// Events
void Sys_QueEvent(int time, enum SysEventType type, int value, int value2, int data_len, void *data);


LRESULT WINAPI MainWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
struct WinVars {
	void *			wndproc;
	HINSTANCE		reflib_library;		// Handle to refresh DLL 
	b32				reflib_active;

	HWND			hwnd;
	HINSTANCE		hinstance;
	b32				active_app;
	b32				is_minimized;
	OSVERSIONINFO	os_version;

	// when we get a windows message, we store the time off so keyboard processing
	// can know the exact time of an event
	u32				sys_msg_time;
};
extern WinVars global_win_vars;
#endif	// Header guard
