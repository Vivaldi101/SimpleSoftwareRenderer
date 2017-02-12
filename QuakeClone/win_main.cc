#include "win_shared.h"
#include "common.h"
#include "win_local.h"
#include "plg_loader.h"

static int	s_global_sys_argc;
static char *s_global_sys_argv[MAX_NUM_ARGVS];

WinVars global_win_vars;
static WINDOWPLACEMENT global_window_pos = { sizeof(global_window_pos) };

void Sys_Init() {
	// does all the system specific init stuff
}

void Sys_Quit() {
	Sys_DestroyConsole();
	// FIXME: maybe free the list and stack allocators
	timeEndPeriod(1);
	exit(0);
}

void Sys_Print(const char *msg) {
	Con_AppendText(msg);
}

void Sys_Sleep(DWORD ms) {
	Sleep(ms);
}

void Sys_ToggleFullscreen(HWND window) {
	DWORD style = GetWindowLong(window, GWL_STYLE);
	if (style & WS_OVERLAPPEDWINDOW) {
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(window, &global_window_pos) && GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi)) {
			SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window, HWND_TOP,
						 mi.rcMonitor.left, mi.rcMonitor.top,
						 mi.rcMonitor.right - mi.rcMonitor.left,
						 mi.rcMonitor.bottom - mi.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	} else {
		SetWindowLong(window, GWL_STYLE,
					  style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &global_window_pos);
		SetWindowPos(window, 0, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

static void ParseCommandLine(char *cmd_line) {
	s_global_sys_argc = 1;
	s_global_sys_argv[0] = "exe";

	while (*cmd_line && (s_global_sys_argc < MAX_NUM_ARGVS)) {
		while (*cmd_line && ((*cmd_line <= 32) || (*cmd_line > 126))) {
			++cmd_line;
		}

		if (*cmd_line) {
			s_global_sys_argv[s_global_sys_argc] = cmd_line;
			++s_global_sys_argc;

			while (*cmd_line && ((*cmd_line > 32) && (*cmd_line <= 126))) {
				cmd_line++;
			}

			if (*cmd_line) {
				*cmd_line = 0;
				++cmd_line;
			}
		}
	}
}

// System event queue
#define	MAX_SYS_QUED_EVENTS		 256
#define	MASK_SYS_QUED_EVENTS	(MAX_SYS_QUED_EVENTS - 1)

static SysEvent global_sys_event_queue[MAX_SYS_QUED_EVENTS];
static int global_sys_event_head, global_sys_eventail;

void Sys_QueEvent(int time, SysEventType ev_type, int value, int value2, int data_len, void *data) {
	SysEvent *ev = &global_sys_event_queue[global_sys_event_head & MASK_SYS_QUED_EVENTS];

	if ( global_sys_event_head - global_sys_eventail >= MAX_SYS_QUED_EVENTS ) {
		//Com_Printf("Sys_QueEvent: overflow\n");
		//// we are discarding an event, but don't leak memory
		//if ( ev->evPtr ) {
		//	Z_Free( ev->evPtr );
		//}
		global_sys_eventail++;
	}

	global_sys_event_head++;

	if (time == 0) {
		time = Sys_GetMilliseconds();
	}

	ev->ev_time = time;
	ev->ev_type = ev_type;
	ev->ev_value = value;
	ev->ev_value2 = value2;
	ev->ev_data_len = data_len;
	ev->ev_data = data;
}

SysEvent Sys_GetEvent() {
	SysEvent	se;

	//char		*s;
	//MSG		msg;

	// return if we have data
	if (global_sys_event_head > global_sys_eventail) {
		global_sys_eventail++;
		return global_sys_event_queue[(global_sys_eventail - 1) & MASK_SYS_QUED_EVENTS];
	}

	// Create an empty event 
	memset(&se, 0, sizeof(se));
	se.ev_time = Sys_GetMilliseconds();

	return se;
}

void Sys_PumpEvents() {
    MSG msg;

	// pump the message loop
	while (PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage( &msg, NULL, 0, 0)) {
			Sys_Quit();
		}

		// save the msg time, because wndprocs don't have access to the timestamp
		if (global_win_vars.sys_msg_time && global_win_vars.sys_msg_time > (int)msg.time) {
			// don't ever let the event times run backwards	
//			common->Printf( "Sys_PumpEvents: win32.sysMsgTime (%i) > msg.time (%i)\n", win32.sysMsgTime, msg.time );
		} else {
			global_win_vars.sys_msg_time = msg.time;
		}

		TranslateMessage(&msg);
      	DispatchMessage(&msg);
	}
}

void Sys_GenerateEvents() {
	static int entered = false;
	//char *s;

	if (entered) {
		return;
	}
	entered = true;

	// pump the message loop
	Sys_PumpEvents();

	// grab or release the mouse cursor if necessary
#if 0
	IN_Frame();

	// check for console commands
	s = Sys_ConsoleInput();
	if ( s ) {
		char	*b;
		int		len;

		len = strlen( s ) + 1;
		b = (char *)Mem_Alloc( len, TAG_EVENTS );
		strcpy( b, s );
		Sys_QueEvent( SE_CONSOLE, 0, 0, len, b, 0 );
	}

#endif
	entered = false;
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show) {

	// just for prototyping purposes, will be moved elsewhere
	global_win_vars.hinstance = hinstance;
	global_win_vars.wndproc = MainWndProc;

	ParseCommandLine(cmd_line);
	Sys_CreateConsole();
	Sys_ShowConsole(1, true);

	// Initial time resolution and base
	timeBeginPeriod(1);
	Sys_GetMilliseconds();

	EngineData ed = Com_InitEngine(global_win_vars.hinstance, global_win_vars.wndproc);

	for (;;) {
		//startime = Sys_GetMilliseconds();
		// Run 1 frame of input
		//In_Frame();
		// Run 1 frame of update and render
		Com_RunFrame(&ed);
		//endime = Sys_GetMilliseconds();
		//globalotal_msec += (endime - startime);
		//global_frame_counter++;
	}
	//Com_Quit();
}
