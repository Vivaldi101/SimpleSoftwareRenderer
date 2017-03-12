#ifndef COMMON_H
#define COMMON_H
#include "shared.h"		// FIXME: merge shared and common.h into one
#include "renderer_types.h"
#include "keys.h"

#define MAX_UPS 120
#define MSEC_PER_SIM (1000 / MAX_UPS)

struct Input {
	Key	keys[MAX_NUM_KEYS];
};

struct Platform {
	struct Renderer *		renderer;
	ListAllocator *			list_allocator;
	// FIXME: get the stack allocator from other project
	MemoryStack *			perm_data;
	MemoryStack *			temp_data; 
	Input		*			input;
};

enum SysEventType {
	SET_NONE = 0,		// evTime is still valid
	SET_KEY,				// evValue is a key code, evValue2 is the down flag
	SET_CHAR,			// evValue is an ascii char
	SET_MOUSE,			// evValue and evValue2 are reletive signed x / y moves
	SET_CONSOLE,			// evPtr is a char*
	SET_PACKET			// evPtr is a netadr followed by data bytes to evPtrLength
};

struct SysEvent {
	int					ev_time;
	enum SysEventType	ev_type;
	int					ev_value, ev_value2;
	int					ev_data_len;			
	void *				ev_data;				
};

// Console
extern void Sys_ShowConsole(int level, b32 quit_on_close);
extern void Sys_Print(const char *msg);

// Timing
extern int Sys_GetMilliseconds();
static inline int Com_ModifyFrameMsec(int frame_msec);

// Common
extern Platform Com_Init(void *hinstance, void *wndproc);
extern void Com_RunFrame(Platform *pf);
extern void Com_Quit();
void *Allocate(ListAllocator *la, size_t num_bytes);
void Free(ListAllocator *la, void **ptr);

// Events
extern void Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();

// Input
extern void IN_ClearKeyStates(Key *keys);

#endif	// Header guard
