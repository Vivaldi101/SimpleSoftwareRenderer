#include "win_local.h"
#include "common.h"

#define COPY_ID			1
#define QUIT_ID			2
#define CLEAR_ID		3

#define EDIT_ID			100
#define INPUT_ID		101

struct WinConData {
	HWND		hwnd;
	HWND		hwnd_buffer;

	HWND		hwnd_buttonclear;
	HWND		hwnd_buttoncopy;
	HWND		hwnd_buttonquit;

	HWND		hwnd_errorbox;
	HWND		hwnd_errortext;

	HBITMAP		hbm_logo;
	HBITMAP		hbm_clearbitmap;

	HBRUSH		hbr_edit_background;
	HBRUSH		hbr_error_background;

	HFONT		hf_buffer_font;
	HFONT		hf_button_font;

	HWND		hwnd_inputline;

	char		error_string[80];

	char		consoleext[512], returnedext[512];
	int			vis_level;
	b32	quit_on_close;
	int			window_width, window_height;
	
	WNDPROC		sys_input_line_wndproc;

};
static WinConData s_global_wcd;
static LRESULT WINAPI ConWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
#define TIMER_ID 1
	char *cmd_string;
	static b32 sime_polarity = false;

	switch (umsg) {
		case WM_ACTIVATE: {
			if (LOWORD(wparam) != WA_INACTIVE) {
				SetFocus(s_global_wcd.hwnd_inputline);
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
			if (s_global_wcd.quit_on_close) {
				Com_Quit();
			} else {
				Sys_ShowConsole(0, false);
				Com_Quit();
				//Cvar_Set( "viewlog", "0" );
			}
			return 0;
		}
		case WM_CTLCOLORSTATIC: {
			if ((HWND)lparam == s_global_wcd.hwnd_buffer) {
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
				return (LRESULT)s_global_wcd.hbr_edit_background;
			} else if ((HWND)lparam == s_global_wcd.hwnd_errorbox ) {
				if (sime_polarity & 1) {
					SetBkColor((HDC)wparam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor((HDC)wparam, RGB( 0xff, 0x0, 0x00 ) );
				} else {
					SetBkColor((HDC)wparam, RGB( 0x80, 0x80, 0x80 ) );
					SetTextColor((HDC)wparam, RGB( 0x00, 0x0, 0x00 ) );
				}
				return (LRESULT)s_global_wcd.hbr_error_background;
			} 
		} break;
	#if 1
		//case WM_COMMAND: {
		//	if (wparam == COPY_ID) {
		//		SendMessage(s_global_wcd.hwnd_buffer, EM_SETSEL, 0, -1 );
		//		SendMessage(s_global_wcd.hwnd_buffer, WM_COPY, 0, 0 );
		//	} else if (wparam == QUIT_ID) {
		//		if (s_global_wcd.quit_on_close) {
		//			PostQuitMessage(0);
		//		} else {
		//			//cmdstring = copystring( "quit" );
		//			//Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdstring ) + 1, cmdstring );
		//		}
		//	} else if (wparam == CLEAR_ID) {
		//		SendMessage(s_global_wcd.hwnd_buffer, EM_SETSEL, 0, -1 );
		//		SendMessage(s_global_wcd.hwnd_buffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
		//		UpdateWindow(s_global_wcd.hwnd_buffer);
		//	}
		// } break;
	#endif
	#if 1
		case WM_CREATE: {
			//s_global_wcd.hbm_logo = LoadBitmap(global_win_vars.hinstance, MAKEINTRESOURCE(IDB_BITMAP1));
			//s_global_wcd.hbm_clearbitmap = LoadBitmap(global_win_vars.hinstance, MAKEINTRESOURCE( IDB_BITMAP2 ) );
			s_global_wcd.hbr_edit_background = CreateSolidBrush(RGB( 0x00, 0x00, 0xB0));
			s_global_wcd.hbr_error_background = CreateSolidBrush(RGB( 0xFF, 0x00, 0x00));
			SetTimer(hwnd, TIMER_ID, 1000, NULL);
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
				sime_polarity = (b32)(!sime_polarity);
				if (s_global_wcd.hwnd_errorbox) {
					InvalidateRect(s_global_wcd.hwnd_errorbox, NULL, FALSE);
				}
			}
		} break;
	#endif
    }

    return DefWindowProc( hwnd, umsg, wparam, lparam );
}

void Con_AppendText(const char *msg) {
#define CONSOLE_BUFFER_SIZE	16384
	char buffer[CONSOLE_BUFFER_SIZE * 2];	// why is it multiplied by two

	char *b = buffer;
	const char *temp_msg = msg;
	int i = 0;

	static u32 total_chars;
	u32 buf_len;

	//
	// if the message is REALLY long, use just the last portion of it
	//
	//if ( strlen( pMsg ) > CONSOLE_BUFFER_SIZE - 1 ) {
	//	msg = pMsg + strlen( pMsg ) - CONSOLE_BUFFER_SIZE + 1;
	//}
	//else {
	//	msg = pMsg;
	//}

	//
	// copy into an intermediate buffer
	//
	while (temp_msg[i] && ((b - buffer) < sizeof(buffer) - 1)) {
		if (temp_msg[i] == '\n' && temp_msg[i+1] == '\r') {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		}
		else if (temp_msg[i] == '\r') {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if (temp_msg[i] == '\n') {
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		//else if ( Q_IsColorString( &temp_msg[i] ) )
		//{
		//	i++;
		//}
		else {
			*b = temp_msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	buf_len = (u32)(b - buffer);

	total_chars += buf_len;

	//
	// replace selection instead of appending if we're overflowing
	//
	if (total_chars > 0x7fff) {
		SendMessage( s_global_wcd.hwnd_buffer, EM_SETSEL, 0, -1 );
		total_chars = buf_len;
	}

	//
	// put this text into the windows console
	//
	//SendMessage(s_global_wcd.hwnd_buffer, EM_LINESCROLL, 0, 0xffff); this is not needed by bro
	SendMessage(s_global_wcd.hwnd_buffer, EM_SCROLLCARET, 0, 0 );
	SendMessage(s_global_wcd.hwnd_buffer, EM_REPLACESEL, 0, (LPARAM)buffer);
}
#if 1
void Sys_CreateConsole() {
	WNDCLASS wc = {};
	HDC hdc;
	RECT rect;

	const char *console_class_name = "QC Console";
	int nheight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;

	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc   = ConWndProc;
	wc.hInstance     = global_win_vars.hinstance;
	wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = console_class_name;

	if (!RegisterClass(&wc)) {
		Sys_Quit();
	}
	rect.left = 0;
	rect.right = 540;
	rect.top = 0;
	rect.bottom = 450;
	AdjustWindowRect(&rect, DEDSTYLE, FALSE);

	hdc = GetDC(GetDesktopWindow());
	swidth = GetDeviceCaps(hdc, HORZRES);
	sheight = GetDeviceCaps(hdc, VERTRES);
	ReleaseDC(GetDesktopWindow(), hdc);

	s_global_wcd.window_width = rect.right - rect.left + 1;
	s_global_wcd.window_height = rect.bottom - rect.top + 1;

	s_global_wcd.hwnd = CreateWindowEx(0,
									   console_class_name,
									   "QC Console",
									   DEDSTYLE,
									   1350, 550, /*( swidth - 600 ) / 2, ( sheight - 450 ) / 2 ,*/
									   rect.right - rect.left + 1, rect.bottom - rect.top + 1,
									   NULL,
									   NULL,
									   global_win_vars.hinstance,
									   NULL);

	if (!s_global_wcd.hwnd) {
		Sys_Quit();
	}

	// Generate fonts
	hdc = GetDC(s_global_wcd.hwnd);
	nheight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	s_global_wcd.hf_buffer_font = CreateFont(nheight,
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

	ReleaseDC(s_global_wcd.hwnd, hdc);
	
	// Create the input line
	s_global_wcd.hwnd_inputline = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | 
												ES_LEFT | ES_AUTOHSCROLL,
												6, 400, 528, 20,
												s_global_wcd.hwnd, 
												(HMENU)INPUT_ID,	// child input line window ID
												global_win_vars.hinstance, NULL);

	// Create the scrollbuffer
	s_global_wcd.hwnd_buffer = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | 
											ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
											6, 40, 526, 354,
											s_global_wcd.hwnd, 
											(HMENU)EDIT_ID,			// child scrollbar window ID
											global_win_vars.hinstance, NULL );



	//ShowWindow(s_global_wcd.hwnd, SW_SHOWDEFAULT);
	//UpdateWindow(s_global_wcd.hwnd);
	//SetForegroundWindow(s_global_wcd.hwnd);
	//SetFocus(s_global_wcd.hwnd_inputline);

	//s_global_wcd.vis_level = 1;
}

void Sys_ShowConsole(int vis_level, b32 quit_on_close) {
	s_global_wcd.quit_on_close = quit_on_close;

	if (vis_level == s_global_wcd.vis_level) {
		return;
	}

	s_global_wcd.vis_level = vis_level;

	if (!s_global_wcd.hwnd) {
		return;
	}

	switch (vis_level) {
		case 0: {
			ShowWindow(s_global_wcd.hwnd, SW_HIDE);
		} break;
		case 1: {
			ShowWindow(s_global_wcd.hwnd, SW_SHOWNORMAL);
			SendMessage(s_global_wcd.hwnd_buffer, EM_LINESCROLL, 0, 0xffff );
			Sys_Print("\nConsole created\n");
		} break;
		case 2:
			ShowWindow(s_global_wcd.hwnd, SW_MINIMIZE); {
			Sys_Print("Console minimized");
		} break;
		default: {
			Sys_Print("\nInvalid vis_level sent to Sys_ShowConsole\n");
		} break;
	}
}
void Sys_DestroyConsole() {
	if (s_global_wcd.hwnd) {
		ShowWindow(s_global_wcd.hwnd, SW_HIDE);
		CloseWindow(s_global_wcd.hwnd);
		DestroyWindow(s_global_wcd.hwnd);
		s_global_wcd.hwnd = 0;
	}
}
#endif

