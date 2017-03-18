#include "win_local.h"
#include "common.h"

#if 0
static int MapKey (int key) {
	int result;
	int modified;
	b32 is_extended;

//	Com_Printf( "0x%x\n", key);

	modified = ( key >> 16 ) & 255;

	if ( modified > 127 )
		return 0;

	if ( key & ( 1 << 24 ) )
	{
		is_extended = qtrue;
	}
	else
	{
		is_extended = qfalse;
	}

	result = s_scantokey[modified];

	if ( !is_extended )
	{
		switch ( result )
		{
		case K_HOME:
			return K_KP_HOME;
		case K_UPARROW:
			return K_KP_UPARROW;
		case K_PGUP:
			return K_KP_PGUP;
		case K_LEFTARROW:
			return K_KP_LEFTARROW;
		case K_RIGHTARROW:
			return K_KP_RIGHTARROW;
		case K_END:
			return K_KP_END;
		case K_DOWNARROW:
			return K_KP_DOWNARROW;
		case K_PGDN:
			return K_KP_PGDN;
		case K_INS:
			return K_KP_INS;
		case K_DEL:
			return K_KP_DEL;
		default:
			return result;
		}
	}
	else
	{
		switch ( result )
		{
		case K_PAUSE:
			return K_KP_NUMLOCK;
		case 0x0D:
			return K_KP_ENTER;
		case 0x2F:
			return K_KP_SLASH;
		case 0xAF:
			return K_KP_PLUS;
		}
		return result;
	}
}
#endif

LRESULT WINAPI MainWndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
		case WM_CREATE: {
			global_win_vars.hwnd = window;
		} break;

		// fall through 
		// FIXME: not sure about the int casts
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_KEY, (int)wparam, true, 0, 0);
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_KEY, (int)wparam, false, 0, 0);
			break;

		case WM_CHAR:
			if (wparam == 'f') {
				Sys_ToggleFullscreen(window);
			}
			Sys_QueEvent(global_win_vars.sys_msg_time, SET_CHAR, (int)wparam, 0, 0, 0);
			break;
	}

    return DefWindowProc(window, msg, wparam, lparam);
}
