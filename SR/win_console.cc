#include "win_local.h"
#include "common.h"

#define COPY_ID			1
#define QUIT_ID			2
#define CLEAR_ID		3

#define EDIT_ID			100
#define INPUT_ID		101

Console global_console;

static LRESULT WINAPI ConWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
#define TIMER_ID 1
	char *cmd_string;
	static b32 time_polarity = false;

	switch (umsg) {
		case WM_ACTIVATE: {
			if (LOWORD(wparam) != WA_INACTIVE) {
				SetFocus(global_console.hwnd_inputline);
			}
			//if (com_viewlog && (com_dedicated && !com_dedicated->integer)) {
			//	// if the viewlog is open, check to see if it's being minimized
			//	if ( com_viewlog->integer == 1 ) {
			//		if ( HIWORD(wparam) ) {		// minimized flag
			//			//Cvar_Set("viewlog", "2");
			//		}
			//	}
			//	else if ( com_viewlog->integer == 2 ) {
			//		if ( !HIWORD(wparam) ) {		// minimized flag
			//			//Cvar_Set("viewlog", "1");
			//		}
			//	}
			//}
		} break;

		case WM_CLOSE: {
			//if ( ( com_dedicated && com_dedicated->integer ) ) {
			//	cmdstring = copystring( "quit" );
			//	Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdstring ) + 1, cmdstring );
			//}
			if (global_console.quit_on_close) {
				Com_Quit();
			} else {
				Sys_FetchConsole(CON_HIDE, false);
				//Cvar_Set( "viewlog", "0" );
			}
			return 0;
		}
		case WM_CTLCOLORSTATIC: {
			if ((HWND)lparam == global_console.hwnd_buffer) {
				SetBkColor((HDC) wparam, RGB( 0x00, 0x00, 0xB0));
				SetTextColor((HDC) wparam, RGB( 0xff, 0xff, 0x00));

	#if 0	// this draws a background in the edit box, but there are issues with this
				if ( ( hdcScaled = CreateCompatibleDC( ( HDC ) wparam ) ) != 0 )
				{
					if ( SelectObject( ( HDC ) hdcScaled, s_wcd.hbmLogo ) )
					{
						StretchBlt( ( HDC ) wparam, 0, 0, 512, 384, 
								hdcScaled, 0, 0, 512, 384,
								SRCCOPY );
					}
					DeleteDC( hdcScaled );
				}
	#endif
				return (LRESULT)global_console.hbr_edit_background;
			} else if ((HWND)lparam == global_console.hwnd_errorbox ) {
				if (time_polarity & 1) {
					SetBkColor((HDC)wparam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor((HDC)wparam, RGB( 0xff, 0x0, 0x00 ) );
				} else {
					SetBkColor((HDC)wparam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor((HDC)wparam, RGB( 0x00, 0x0, 0x00 ) );
				}
				return (LRESULT)global_console.hbr_error_background;
			} 
		} break;
	#if 1
		//case WM_COMMAND: {
		//	if (wparam == COPY_ID) {
		//		SendMessage(global_console.hwnd_buffer, EM_SETSEL, 0, -1 );
		//		SendMessage(global_console.hwnd_buffer, WM_COPY, 0, 0 );
		//	} else if (wparam == QUIT_ID) {
		//		if (global_console.quit_on_close) {
		//			PostQuitMessage(0);
		//		} else {
		//			//cmdstring = copystring( "quit" );
		//			//Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdstring ) + 1, cmdstring );
		//		}
		//	} else if (wparam == CLEAR_ID) {
		//		SendMessage(global_console.hwnd_buffer, EM_SETSEL, 0, -1 );
		//		SendMessage(global_console.hwnd_buffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
		//		UpdateWindow(global_console.hwnd_buffer);
		//	}
		// } break;
	#endif
	#if 1
		case WM_CREATE: {
			//global_console.hbm_logo = LoadBitmap(global_win_vars.hinstance, MAKEINTRESOURCE(IDB_BITMAP1));
			//global_console.hbm_clearbitmap = LoadBitmap(global_win_vars.hinstance, MAKEINTRESOURCE( IDB_BITMAP2 ) );
			global_console.hbr_edit_background = CreateSolidBrush(RGB( 0x00, 0x00, 0xB0));
			global_console.hbr_error_background = CreateSolidBrush(RGB( 0xFF, 0x00, 0x00));
			SetTimer(hwnd, TIMER_ID, 1000, 0);
		} break;
	#endif
	#if 1
	#if 0
		case WM_ERASEBKGND:
		HDC hdcScaled;
		HGDIOBJ oldObject;

	#if 1	// a single, large image
			hdcScaled = CreateCompatibleDC( ( HDC ) wparam );
			Assert( hdcScaled != 0 );

			if ( hdcScaled )
			{
				oldObject = SelectObject( ( HDC ) hdcScaled, s_wcd.hbmLogo );
				Assert( oldObject != 0 );
				if ( oldObject )
				{
					StretchBlt( ( HDC ) wparam, 0, 0, s_wcd.windowWidth, s_wcd.windowHeight, 
							hdcScaled, 0, 0, 512, 384,
							SRCCOPY );
				}
				DeleteDC( hdcScaled );
				hdcScaled = 0;
			}
	#else	// a repeating brush
			{
				HBRUSH hbrClearBrush;
				RECT r;

				GetWindowRect( hwnd, &r );

				r.bottom = r.bottom - r.top + 1;
				r.right = r.right - r.left + 1;
				r.top = 0;
				r.left = 0;

				hbrClearBrush = CreatePatternBrush( s_wcd.hbmClearBitmap );

				Assert( hbrClearBrush != 0 );

				if ( hbrClearBrush )
				{
					FillRect( ( HDC ) wparam, &r, hbrClearBrush );
					DeleteObject( hbrClearBrush );
				}
			}
	#endif
			return 1;
			return DefWindowProc( hwnd, umsg, wparam, lparam );
	#endif
		case WM_TIMER: {
			if (wparam == TIMER_ID) {
				time_polarity = (b32)(!time_polarity);
				if (global_console.hwnd_errorbox) {
					InvalidateRect(global_console.hwnd_errorbox, 0, FALSE);
				}
			}
		} break;
	#endif
    }

    return DefWindowProc( hwnd, umsg, wparam, lparam );
}

void Con_AppendText(const char *msg) {
	static u32 total_chars;
	char buffer[1 << 12];	
	char *b = buffer;
	const char *temp_msg = msg;
	int i = 0;

	// copy into an intermediate buffer
	while (temp_msg[i] && ((b - buffer) < sizeof(buffer) - 1)) {
		if (temp_msg[i] == '\n' && temp_msg[i+1] == '\r') {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		} else if (temp_msg[i] == '\r') {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		} else if (temp_msg[i] == '\n') {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		} else {
			*b = temp_msg[i];
			b++;
		}
		i++;
	}

	*b = 0;
	Assert((b - buffer) >= 0);
	u32 buf_len = (u32)(b - buffer);
	total_chars += buf_len;

	if (total_chars > (1 << 12)) {
		SendMessage(global_console.hwnd_buffer, EM_SETSEL, 0, -1);
		total_chars = buf_len;
	}

	SendMessage(global_console.hwnd_buffer, EM_SCROLLCARET, 0, 0);
	SendMessage(global_console.hwnd_buffer, EM_REPLACESEL, 0, (LPARAM)buffer);
}
#if 1
void Sys_CreateConsole(HINSTANCE hinstance) {
	WNDCLASS wc = {};
	HDC hdc;
	RECT rect;

	const char *console_class_name = "Debug Console";
	int nheight;
	int swidth, sheight;
	int style_bits = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;

	wc.lpfnWndProc   = ConWndProc;
	wc.hInstance     = hinstance;
	wc.hCursor       = LoadCursor (0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = console_class_name;

	if (!RegisterClass(&wc)) {
		Com_Quit();
	}
	rect.left = 0;
	rect.right = 540;
	rect.top = 0;
	rect.bottom = 450;
	AdjustWindowRect(&rect, style_bits, FALSE);

	hdc = GetDC(GetDesktopWindow());
	swidth = GetDeviceCaps(hdc, HORZRES);
	sheight = GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(GetDesktopWindow(), hdc);

	global_console.window_width = rect.right - rect.left + 1;
	global_console.window_height = rect.bottom - rect.top + 1;

	global_console.hwnd = CreateWindowEx(0,
									   console_class_name,
									   "Console",
									   style_bits,
									   1365, 600, 
									   rect.right - rect.left + 1, rect.bottom - rect.top + 1,
									   0,
									   0,
									   hinstance,
									   0);

	if (!global_console.hwnd) {
		Com_Quit();
	}

	// generate fonts
	hdc = GetDC(global_console.hwnd);
	nheight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	global_console.hf_buffer_font = CreateFont( nheight,
												0,
											 	0,
											 	0,
											 	FW_LIGHT,
											 	0,
											 	0,
											 	0,
											 	DEFAULT_CHARSET,
											 	OUT_DEFAULT_PRECIS,
											 	CLIP_DEFAULT_PRECIS,
											 	CLEARTYPE_QUALITY,
											 	FF_MODERN | FIXED_PITCH,
											 	"Courier New");

	ReleaseDC(global_console.hwnd, hdc);
	
	// create the input line
	global_console.hwnd_inputline = CreateWindow("edit", 0, WS_CHILD | WS_VISIBLE | WS_BORDER | 
												 ES_LEFT | ES_AUTOHSCROLL,
												 6, 400, 528, 20,
												 global_console.hwnd, 
												 (HMENU)INPUT_ID,	// child input line window ID
												 hinstance, 0);

	// create the scrollbuffer
	global_console.hwnd_buffer = CreateWindow("edit", 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | 
											  ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
											  6, 40, 526, 354,
											  global_console.hwnd, 
											  (HMENU)EDIT_ID,			// child scrollbar window ID
											  hinstance, 0 );
}

void Sys_FetchConsole(ConsoleVis visibility, b32 quit_on_close) {
	if (!global_console.hwnd) {
		return;
	}

	global_console.quit_on_close = quit_on_close;
	if (visibility == global_console.visibility) {
		return;
	}
	global_console.visibility = visibility;

	switch (visibility) {
		case CON_HIDE: {
			ShowWindow(global_console.hwnd, SW_HIDE);
		} break;
		case CON_SHOW: {
			SetWindowPos(global_console.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			SendMessage(global_console.hwnd_buffer, EM_LINESCROLL, 0, 0xffff );
		} break;
		case CON_MINIMIZE: {
			ShowWindow(global_console.hwnd, SW_MINIMIZE);
		} break;
		default: {
			Sys_Print("\nInvalid vis_level sent to Sys_ShowConsole\n");
		} break;
	}
}

void Sys_DestroyConsole() {
	if (global_console.hwnd) {
		ShowWindow(global_console.hwnd, SW_HIDE);
		CloseWindow(global_console.hwnd);
		DestroyWindow(global_console.hwnd);
		global_console.hwnd = 0;
	}
}
#endif

