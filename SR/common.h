#ifndef COMMON_H
#define COMMON_H
#include "shared.h"		
#include "input.h"

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

struct Bitmap {
   union {
      Vec2i dim;
      struct {
         s32 w, h;
      };
   };
	byte *	data;		
};

struct GameState {
	struct BaseEntity 	*entities;
	int						num_base_entities;

	Bitmap					texts[32];
	int						num_texts;
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
	StackAllocator 	main_memory_stack;
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
	int					type;
	int					value, value2;
	int					data_size;			
	void *				data;				
};

// Console
enum ConsoleVis {
	CON_HIDE = 0,
	CON_SHOW,
	CON_MINIMIZE
};
struct Console {
	HWND			hwnd;
	HWND			hwnd_buffer;

	HWND			hwnd_buttonclear;
	HWND			hwnd_buttoncopy;
	HWND			hwnd_buttonquit;

	HWND			hwnd_errorbox;
	HWND			hwnd_errortext;

	HBITMAP			hbm_logo;
	HBITMAP			hbm_clearbitmap;

	HBRUSH			hbr_edit_background;
	HBRUSH			hbr_error_background;

	HFONT			hf_buffer_font;
	HFONT			hf_button_font;

	HWND			hwnd_inputline;

	char			error_string[80];

	char			console_text[512], returned_text[512];
	ConsoleVis		visibility;
	b32				quit_on_close;
	int				window_width, window_height;
	
	WNDPROC			sys_input_line_wndproc;

};

extern Console global_console;
extern void	Sys_CreateConsole(HINSTANCE hinstance);
extern void	Sys_DestroyConsole();
extern void Con_AppendText(const char *msg);
extern void Sys_FetchConsole(ConsoleVis visibility, b32 quit_on_close);

// Timing
extern int Sys_GetMilliseconds();
//static inline int Com_ModifyFrameMsec(int frame_msec);

// Common
struct Renderer;
extern void Com_Allocate(Platform **pf, Renderer **ren);
extern void Com_SetupIO(Platform *pf);
extern void Com_LoadEntities(Platform *pf);
extern void Com_LoadTextures(Platform *pf, Renderer *r);
extern void Com_LoadFonts(Platform *pf, Renderer *r);
extern bool Com_RunFrame(Platform *pf, Renderer *ren);
extern void Com_Quit();

// Events
extern bool Sys_GenerateEvents();
extern struct SysEvent Com_GetEvent();
extern struct SysEvent Sys_GetEvent();

// Input
extern void IN_GetInput(Input *in);
extern void IN_HandleInput(Input *in);
extern void IN_ClearKeys(Input *in);

// Strings
extern int StrCmp(const char* a, const char* b);

// Fonts
extern Bitmap TTF_LoadGlyph(MemoryStack *ms, const FileInfo *ttf_file, int code_point);
extern Bitmap TTF_LoadString(MemoryStack *ms, const FileInfo *ttf_file, const char *str);

// Misc
extern Bitmap MakeBitmap(MemoryStack *ms, int width, int height);
extern u32 PackRGBA(Vec4 color);
extern u32 PackRGBA(r32 r, r32 g, r32 b, r32 a);
extern Vec4 UnpackRGBA(u32 color);
#endif	// Header guard
