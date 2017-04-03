#ifndef COMMON_H
#define COMMON_H
#include "shared.h"		
#include "r_types.h"
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
	Key keys[MAX_NUM_KEYS];
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
		Vec3	dir;
		Vec3	world_pos;
		Vec3	orientation;
		Vec3	velocity;

		int		num_verts;
		int		num_polys;
	} status;	// FIXME: remove the status var

	// FIXME: remove
	Mesh *	mesh;
};

struct GameState {
	Entity 		entities[MAX_NUM_ENTITIES];
	int			num_entities;
};

struct StackAllocator {
	// FIXME: make a single double ended stack, 
	// with temp and perm allocations coming from the opposite sides
	MemoryStack * 	perm_data;
	MemoryStack * 	temp_data; 
};

struct Platform {
	StackAllocator 		main_memory_stack;
	Input				input_state;
	GameState	*		game_state;
};

enum SysEventType {
	SET_NONE = 0,		
	SET_KEY,			
	SET_CHAR,			
	SET_MOUSE,			
	SET_CONSOLE			
};

struct SysEvent {
	u32					time;
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
extern void Com_RunFrame(Platform *pf, struct RenderingSystem *rs);
extern void Com_Quit();

// Events
extern void Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();

// Input
extern void IN_UpdateKeyboard(Input *in);
extern void IN_ClearKeys(Input *in);

#endif	// Header guard
