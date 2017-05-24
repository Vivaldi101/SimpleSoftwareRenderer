#ifndef COMMON_H
#define COMMON_H
#include "shared.h"		
#include "r_types.h"
#include "input.h"

#define MAX_UPS 120
#define MSEC_PER_SIM (1000 / MAX_UPS)
#define PLATFORM_FULLSCREEN

#undef MAX_PERM_MEMORY
#undef MAX_TEMP_MEMORY
#define MAX_PERM_MEMORY MEGABYTES(256) 
#define MAX_TEMP_MEMORY MEGABYTES(64) 

#ifdef PLATFORM_FULLSCREEN
#define WINDOW_WIDTH	1920
#define WINDOW_HEIGHT	1080
#else
#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
#endif	// PLATFORM_FULLSCREEN

// DebugFileIO
#ifdef PLATFORM_DEBUG
struct FileInfo {
	void *	data;
	u32		size;
};
#define Debug_FreeFile(name) void name(FileInfo *fi)
typedef Debug_FreeFile(_DebugFreeFile_);

#define Debug_ReadFile(name) FileInfo name(const char *file_name)
typedef Debug_ReadFile(_DebugReadFile_);

#define Debug_WriteFile(name) b32 name(const char *file_name, u32 memory_size, void *memory)
typedef Debug_WriteFile(_DebugWriteFile_);
#endif	// PLATFORM_DEBUG

struct Input {
	Key keys[MAX_NUM_KEYS];
};

// FIXME: move into entities.h 
//#define SetupEntity(table, e, fn) (table)[EntityTypeEnum::##e].fn = (fn##e)
enum EntityTypeEnum {
	EntityType_invalid,
	EntityType_player,
	EntityType_cube,
	MAX_NUM_ENTITY_TYPES
};

static const char *global_entity_names[MAX_NUM_ENTITY_TYPES] = {
	"invalid",
	"player",
	"cube",
};

struct Entity {
	union {
		// FIXME: is this a good way to store vertex arrays?
		// FIXME: hardcoded arrays for now, must match the vertex and poly numbers in the asset file
		struct {
			Poly 	polys[12];
			Vec3	local_vertex_array[8];		
			Vec3	trans_vertex_array[8];		
		} cube;		
		struct {
			Poly 	polys[12];
			Vec3	local_vertex_array[8];		
			Vec3	trans_vertex_array[8];		
		} player;
	};

	EntityTypeEnum type_enum;
	struct {
		int		guid;
		char	type_name[32];

		u16		state;
		u16		attr;

		r32		avg_radius;
		r32		max_radius;

		r32		world_matrix[4][4];	// currently unused
		Vec3	axis[3];			// rotation vectors
		Vec3	world_pos;
		Vec3	orientation;
		Vec3	velocity;

		s16		num_verts;
		s16		num_polys;
	} status;	
};

struct GameState {
	Entity 		entities[MAX_NUM_ENTITIES];
	int			num_entities;

	Bitmap		test_font[MAX_NUM_GLYPHS];
};

struct StackAllocator {
	// FIXME: make a single double ended stack, 
	// with temp and perm allocations coming from the opposite sides
	MemoryStack  	perm_data;
	MemoryStack  	temp_data; 
};

struct FileIO {
	_DebugFreeFile_ *	free_file;
	_DebugReadFile_ *	read_file;
	_DebugWriteFile_ *	write_file;
};

struct Platform {
	StackAllocator 		main_memory_stack;
	Input *				input_state;
	GameState *			game_state;
	FileIO				file_ptrs;
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
extern void Sys_FetchConsole(enum ConVisibility vis_level, b32 quit_on_close);
extern void Sys_Print(const char *msg);

// Timing
extern int Sys_GetMilliseconds();
//static inline int Com_ModifyFrameMsec(int frame_msec);

// Common
extern Platform Com_Init();
extern void Com_LoadEntities(Platform *pf);
extern void Com_RunFrame(Platform *pf, struct Renderer *rs);
extern void Com_Quit();

// Events
extern void Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();

// Input
extern void IN_GetInput(Input *in);
extern void IN_ClearKeys(Input *in);

// Strings
extern int StrCmp(const char* a, const char* b);

// Fonts
extern Bitmap TTF_Init(MemoryStack *ms, const FileInfo *ttf_file, int code_point);

// Misc
extern Bitmap MakeBitmap(MemoryStack *ms, int width, int height);
extern u32 PackRGBA(Vec4 color);
#endif	// Header guard
