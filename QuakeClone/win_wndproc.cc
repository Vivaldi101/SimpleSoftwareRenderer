#include "win_local.h"
#include "common.h"

LRESULT WINAPI MainWndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;
	switch (msg) {
		case WM_SYSKEYDOWN:
		// fall through 
		case WM_KEYDOWN:
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_KEY, (int)wparam, true, 0, 0);
			break;

		case WM_SYSKEYUP:
		// fall through 
		case WM_KEYUP:
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_KEY, (int)wparam, false, 0, 0);
			break;

		case WM_CHAR:
			if (wparam == 'f') {
				Sys_ToggleFullscreen(window);
			} else {
				Sys_QueEvent(global_win_vars.sys_msg_time, SET_CHAR, (int)wparam, 0, 0, 0);
			}
			break;
		default:
			result = DefWindowProc(window, msg, wparam, lparam); 
			break;
	}

    return result;
}
