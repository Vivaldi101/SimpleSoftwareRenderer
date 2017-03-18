#ifndef COMMON_H
#define COMMON_H
#include "shared.h"		
#include "renderer_types.h"
#include "keys.h"

#define MAX_UPS 120
#define MSEC_PER_SIM (1000 / MAX_UPS)
//#define PLATFORM_FULLSCREEN

#ifdef PLATFORM_FULLSCREEN
#define WINDOW_WIDTH	1920
#define WINDOW_HEIGHT	1080
#else
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
#endif	// PLATFORM_FULLSCREEN

struct Input {
	Key	keys[MAX_NUM_KEYS];
};

struct Player {
	Vec3	world_pos;
	Vec3	orientation;
	Vec3	velocity;
	Input *	input;
};

struct GameState {
	MeshObject 	entities[MAX_ENTITIES];
	int			num_entities;
	Player		player;
};

struct Platform {
	struct Renderer *	renderer;
	FBAllocator *		fb_allocator;
	// FIXME: make a single double ended stack, with temp allocations coming from the other side
	MemoryStack *		perm_data;
	MemoryStack *		temp_data; 
	Input		*		input;
	GameState	*		game_state;
};

enum SysEventType {
	SET_NONE = 0,		// evTime is still valid
	SET_KEY,			// evValue is a key code, evValue2 is the down flag
	SET_CHAR,			// evValue is an ascii char
	SET_MOUSE,			// evValue and evValue2 are reletive signed x / y moves
	SET_CONSOLE			// evPtr is a char*
};

struct SysEvent {
	int					time;
	SysEventType		type;
	int					value, value2;
	int					data_size;			
	void *				data;				
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
void *Allocate(FBAllocator *la, size_t num_bytes);
void Free(FBAllocator *la, void **ptr);

// Events
extern void Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();

// Input
extern void IN_HandleKeyEvent(Input *in, int key, b32 down, u32 time);
extern void IN_ClearKeyStates(Input *in);
extern b32 IN_IsKeyDown(Input *in, int key);

#endif	// Header guard
