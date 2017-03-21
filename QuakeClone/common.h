#ifndef COMMON_H
#define COMMON_H
#include "shared.h"		
#include "renderer_types.h"
#include "keys.h"

#define MAX_UPS 60
#define MSEC_PER_SIM (1000 / MAX_UPS)
//#define PLATFORM_FULLSCREEN

#undef MAX_PERM_MEMORY
#undef MAX_TEMP_MEMORY
#define MAX_PERM_MEMORY MEGABYTES(16) 
#define MAX_TEMP_MEMORY MEGABYTES(64) 

#ifdef PLATFORM_FULLSCREEN
#define WINDOW_WIDTH	1920
#define WINDOW_HEIGHT	1080
#else
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
#endif	// PLATFORM_FULLSCREEN

struct Input {
	Key	keys[MAX_NUM_KEYS];
	int curr_key;
	int prev_key;
};

struct Entity {
	struct {
		int		guid;
		char	name[64];

		int		state;
		int		attr;

		r32		avg_radius;
		r32		max_radius;

		//r32	world_matrix[4][4];	// currently unused
		Vec3	world_pos;
		Vec3	orientation;
		Vec3	velocity;

		int		num_verts;
		int		num_polys;
	} status;

	union {
		struct {
		} player;
	};

	Mesh *	mesh;
};

struct GameState {
	Entity 		entities[MAX_ENTITIES];
	int			num_entities;
	//Player		player;
};

struct StackAllocator {
	// FIXME: make a single double ended stack, 
	MemoryStack * 	perm_data;
	MemoryStack * 	temp_data; 
};

struct Platform {
	// fixed block allocator
	//FBAllocator 		fb_allocator;
	// with temp and perm allocations coming from the opposite sides
	StackAllocator 		stack_allocator;
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
extern void Com_RunFrame(Platform *pf, struct Renderer *ren);
extern void Com_SimFrame(r32 dt);
extern void Com_Quit();
void *Allocate(FBAllocator *la, size_t num_bytes);
void Free(FBAllocator *la, void **ptr);

// Events
extern void Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();

// Input
extern b32 IN_IsKeyDown(Input *in, int key);
extern void IN_HandleKeyEvent(Input *in, int key, b32 down, u32 time);
extern void IN_ClearKeyStates(Input *in);

#endif	// Header guard
