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

LRESULT WINAPI MainWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	//static b32 flip = true;
	//int zdelta, i;

	// NOTE: not sure how reliable this is anymore, might trigger double wheel events
#if 0
	if (in_mouse->integer != 1)
	{
		if ( uMsg == MSH_MOUSEWHEEL )
		{
			if ( ( ( int ) wParam ) > 0 )
			{
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
			}
			else
			{
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
			}
			return DefWindowProc (hWnd, uMsg, wParam, lParam);
		}
	}
#endif

	switch (umsg) {
#if 0
	case WM_MOUSEWHEEL:
		// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/mouseinput/aboutmouseinput.asp
		// Windows 98/Me, Windows NT 4.0 and later - uses WM_MOUSEWHEEL
		// only relevant for non-DI input and when console is toggled in window mode
		//   if console is toggled in window mode (KEYCATCH_CONSOLE) then mouse is released and DI doesn't see any mouse wheel
		if (in_mouse->integer != 1 || (!r_fullscreen->integer && (cls.keyCatchers & KEYCATCH_CONSOLE)))
		{
			// 120 increments, might be 240 and multiples if wheel goes too fast
			// NOTE Logitech: logitech drivers are screwed and send the message twice?
			//   could add a cvar to interpret the message as successive press/release events
			zDelta = ( short ) HIWORD( wParam ) / 120;
			if ( zDelta > 0 )
			{
				for(i=0; i<zDelta; i++)
				{
					if (!in_logitechbug->integer)
					{
						Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
						Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
					}
					else
					{
						Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, flip, 0, NULL );
						flip = !flip;
					}
				}
			}
			else
			{
				for(i=0; i<-zDelta; i++)
				{
					if (!in_logitechbug->integer)
					{
						Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
						Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
					}
					else
					{
						Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, flip, 0, NULL );
						flip = !flip;
					}
				}
			}
			// when an application processes the WM_MOUSEWHEEL message, it must return zero
			return 0;
		}
		break;
#endif

	case WM_CREATE: {
		global_win_vars.hwnd = hwnd;

		//vid_xpos = Cvar_Get ("vid_xpos", "3", CVAR_ARCHIVE);
		//vid_ypos = Cvar_Get ("vid_ypos", "22", CVAR_ARCHIVE);
		//r_fullscreen = Cvar_Get ("r_fullscreen", "1", CVAR_ARCHIVE | CVAR_LATCH );

		//MSH_MOUSEWHEEL = RegisterWindowMessage("MSWHEEL_ROLLMSG"); 
		//if ( r_fullscreen->integer )
		//{
		//	WIN_DisableAltTab();
		//}
		//else
		//{
		//	WIN_EnableAltTab();
		//}

	} break;
#if 0
	case WM_DISPLAYCHANGE:
		Com_DPrintf( "WM_DISPLAYCHANGE\n" );
		// we need to force a vid_restart if the user has changed
		// their desktop resolution while the game is running,
		// but don't do anything if the message is a result of
		// our own calling of ChangeDisplaySettings
		if ( com_insideVidInit ) {
			break;		// we did this on purpose
		}
		// something else forced a mode change, so restart all our gl stuff
		Cbuf_AddText( "vid_restart\n" );
		break;
#endif
	case WM_DESTROY: {
		// let sound and input know about this?
		global_win_vars.hwnd = 0;
		//if ( r_fullscreen->integer )
		//{
		//	WIN_EnableAltTab();
		//}
	} break;

	case WM_CLOSE:
		//Cbuf_ExecuteText( EXEC_APPEND, "quit" );
		//break;

	//case WM_ACTIVATE:
	//	{
	//		int	fActive, fMinimized;

	//		fActive = LOWORD(wParam);
	//		fMinimized = (BOOL) HIWORD(wParam);

	//		VID_AppActivate( fActive != WA_INACTIVE, fMinimized);
	//		SNDDMA_Activate();
	//	}
	//	break;

	//case WM_MOVE:
	//	{
	//		int		xPos, yPos;
	//		RECT r;
	//		int		style;

	//		if (!r_fullscreen->integer )
	//		{
	//			xPos = (short) LOWORD(lParam);    // horizontal position 
	//			yPos = (short) HIWORD(lParam);    // vertical position 

	//			r.left   = 0;
	//			r.top    = 0;
	//			r.right  = 1;
	//			r.bottom = 1;

	//			style = GetWindowLong( hWnd, GWL_STYLE );
	//			AdjustWindowRect( &r, style, FALSE );

	//			Cvar_SetValue( "vid_xpos", xPos + r.left);
	//			Cvar_SetValue( "vid_ypos", yPos + r.top);
	//			vid_xpos->modified = qfalse;
	//			vid_ypos->modified = qfalse;
	//			if ( g_wv.activeApp )
	//			{
	//				IN_Activate (qtrue);
	//			}
	//		}
	//	}
	//	break;

// this is complicated because Win32 seems to pack multiple mouse events into
// one update sometimes, so we always check all states and look for events
	//case WM_LBUTTONDOWN:
	//case WM_LBUTTONUP:
	//case WM_RBUTTONDOWN:
	//case WM_RBUTTONUP:
	//case WM_MBUTTONDOWN:
	//case WM_MBUTTONUP:
	//case WM_MOUSEMOVE:
	//	{
	//		int	temp;

	//		temp = 0;

	//		if (wParam & MK_LBUTTON)
	//			temp |= 1;

	//		if (wParam & MK_RBUTTON)
	//			temp |= 2;

	//		if (wParam & MK_MBUTTON)
	//			temp |= 4;

	//		IN_MouseEvent (temp);
	//	}
	//	break;

	//case WM_SYSCOMMAND: {
	//	if (wparam == SC_MINIMIZE) {
	//		exit(0);
	//	}
	//} break;

		case WM_SYSKEYDOWN:
			//if ( wParam == 13 )
			//{
			//	if ( r_fullscreen )
			//	{
			//		Cvar_SetValue( "r_fullscreen", !r_fullscreen->integer );
			//		Cbuf_AddText( "vid_restart\n" );
			//	}
			//	return 0;
			//}

			// fall through
		case WM_KEYDOWN: {
			//Sys_QueEvent(global_win_vars.sys_msgime, SE_KEY, MapKey( lparam ), true, 0, NULL );
		} break;

		case WM_SYSKEYUP:
		case WM_KEYUP: {
			//Sys_QueEvent(global_win_vars.sys_msgime, SE_KEY, MapKey( lparam ), false, 0, NULL );
		} break;

		case WM_CHAR: {
			Sys_QueEvent(global_win_vars.sys_msgime, SE_CHAR, (int)wparam, 0, 0, 0);
		} break;
	}
	

    return DefWindowProc(hwnd, umsg, wparam, lparam);
}
