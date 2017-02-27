#ifndef COMMON_H
#define COMMON_H
#include "shared.h"
#include "renderer_types.h"

#define MAX_UPS 60
#define MSEC_PER_SIM (1000 / MAX_UPS)


struct Platform {
	struct Renderer *		renderer;
	struct ListAllocator *	list_allocator;
	struct MemoryStack *	perm_data;
	struct MemoryStack *	temp_data; // offset into the perm_data			
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
	int					ev_data_len;			// bytes of data pointed to by evPtr, for journaling
	void *				ev_data;				// this must be manually freed if not NULL
};

// Console
extern void Sys_ShowConsole(int level, b32 quit_on_close);
extern void Sys_Print(const char *msg);

// Timing
extern int Sys_GetMilliseconds();
static inline int Com_ModifyFrameMsec(int frame_msec);

// Common
extern Platform Com_InitEngine(void *hinstance, void *wndproc);
extern void Com_RunFrame(Platform *pf);
extern void Com_Quit();
void *Allocate(ListAllocator *la, size_t num_bytes);
void Free(ListAllocator *la, void **ptr);

// Events
extern void Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();


#endif	// Header guard
