#include "win_shared.h"
#include "common.h"
#include "win_local.h"
#include "plg_loader.h"

static int	global_sys_argc;
static char *global_sys_argv[MAX_NUM_ARGVS];

WinVars global_win_vars;
static WINDOWPLACEMENT global_window_pos = { sizeof(global_window_pos) };

void Sys_Init() {
	// does all the system specific init stuff
}

void Sys_Quit() {
	Sys_DestroyConsole();
	timeEndPeriod(1);
	exit(0);
}

void Sys_Print(const char *msg) {
	Con_AppendText(msg);
}

void Sys_Sleep(DWORD ms) {
	Sleep(ms);
}

// from Raymond Chen's blog
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

#if 0
static void ParseCommandLine(char *cmd_line) {
	global_sys_argc = 1;
	global_sys_argv[0] = "exe";

	while (*cmd_line && (global_sys_argc < MAX_NUM_ARGVS)) {
		while (*cmd_line && ((*cmd_line <= 32) || (*cmd_line > 126))) {
			++cmd_line;
		}

		if (*cmd_line) {
			global_sys_argv[global_sys_argc] = cmd_line;
			++global_sys_argc;

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
#endif

//
// event queue
//
#define	MAX_SYS_QUED_EVENTS		256
#define	MASK_SYS_QUED_EVENTS	(MAX_SYS_QUED_EVENTS - 1)

static SysEvent global_sys_event_queue[MAX_SYS_QUED_EVENTS];
static u32 global_sys_event_head, global_sys_event_tail;

void Sys_QueEvent(int time, SysEventType ev_type, int value, int value2, int data_len, void *data) {
	SysEvent *ev = &global_sys_event_queue[global_sys_event_head & MASK_SYS_QUED_EVENTS];

	if (global_sys_event_head - global_sys_event_tail >= MAX_SYS_QUED_EVENTS) {
		EventOverflow;
		global_sys_event_tail++;
	}

	global_sys_event_head++;

	if (time == 0) {
		time = Sys_GetMilliseconds();
	}

	ev->time = time;
	ev->type = ev_type;
	ev->value = value;
	ev->value2 = value2;
	ev->data_size = data_len;
	ev->data = data;
}

SysEvent Sys_GetEvent() {
	SysEvent	se;

	if (global_sys_event_head > global_sys_event_tail) {
		global_sys_event_tail++;
		return global_sys_event_queue[(global_sys_event_tail - 1) & MASK_SYS_QUED_EVENTS];
	}

	// create an empty event 
	memset(&se, 0, sizeof(se));
	se.time = Sys_GetMilliseconds();

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
	// pump the message loop
	Sys_PumpEvents();
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show) {

	Sys_CreateConsole(hinstance);
	Sys_ShowConsole(1, true);

	// initial time resolution
	timeBeginPeriod(1);
	Sys_GetMilliseconds();

	Platform pf = Com_Init(hinstance, MainWndProc);
	Renderer *ren = R_Init(pf.stack_allocator.perm_data, hinstance, MainWndProc);

	for (;;) {
		// run 1 frame of update and render
		Com_RunFrame(&pf, ren);
	}
}
