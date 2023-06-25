#include "win_local.h"
#include "common.h"

bool OnWindowCreate(HWND window, LPCREATESTRUCT c)
{
	return true;
}

void OnWindowPaint(HWND window)
{
	PAINTSTRUCT paint;
	BeginPaint(window, &paint);
	EndPaint(window, &paint);
}

void OnWindowSize(HWND window, UINT state, int cx, int cy)
{
	PostMessage(window, WM_PAINT, 0, 0);
}

LRESULT WINAPI MainWndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	switch (msg) {
		HANDLE_MSG(window, WM_CREATE, OnWindowCreate);
		HANDLE_MSG(window, WM_PAINT, OnWindowPaint);
		HANDLE_MSG(window, WM_SIZE, OnWindowSize);

		case WM_SYSKEYDOWN:
		// fall through 
		case WM_KEYDOWN:
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_KEY, (int)wParam, true, 0, 0);
			break;

		case WM_SYSKEYUP:
		// fall through 
		case WM_KEYUP:
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_KEY, (int)wParam, false, 0, 0);
			break;

		case WM_CHAR:
			if (wParam == 'f') 
			{
				// Handle viewport sizing.
				Sys_ToggleFullscreen(window);
			}
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_CHAR, (int)wParam, 0, 0, 0);
			break;
      case WM_DESTROY:
         Com_Quit();
		 PostQuitMessage(0);
         break;
		default:
			result = DefWindowProc(window, msg, wParam, lParam); 
			break;
	}

    return result;
}
