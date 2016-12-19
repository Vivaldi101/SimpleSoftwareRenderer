#ifndef COMMON_H
#define COMMON_H
#include "shared.h"

#define MAX_UPS 120
#define MSEC_PER_SIM (1000 / MAX_UPS)

// Console
extern void Sys_ShowConsole(int level, b32 quit_on_close);
extern void Sys_Print(const char *msg);

// Timing
extern int Sys_GetMilliseconds();
extern inline int Com_ModifyFrameMsec(int frame_msec);


// Common
extern void Com_Init(int argc, char **argv, void *hinstance, void *wndproc);
extern void Com_Frame();
extern void Com_Quit();

// Events
struct SysEvent;
enum SysEventType;

extern void Sys_GenerateEvents();
extern SysEvent Com_GetEvent();
extern SysEvent Sys_GetEvent();

enum SysEventType {
	SE_NONE = 0,		// evTime is still valid
	SE_KEY,				// evValue is a key code, evValue2 is the down flag
	SE_CHAR,			// evValue is an ascii char
	SE_MOUSE,			// evValue and evValue2 are reletive signed x / y moves
	SE_CONSOLE,			// evPtr is a char*
	SE_PACKET			// evPtr is a netadr followed by data bytes to evPtrLength
};

struct SysEvent {
	int				ev_time;
	SysEventType	ev_type;
	int				ev_value, ev_value2;
	int				ev_data_len;			// bytes of data pointed to by evPtr, for journaling
	void			*ev_data;				// this must be manually freed if not NULL
};
#endif	// Header guard
