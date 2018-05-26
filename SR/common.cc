#include "common.h"
#include "renderer.h"
#include "r_cmds.h"
#include "plg_loader.h"
#include "lights.h"

#define INO_BMP_STATIC
#define INO_BMP_IMPL
#include "ino_bmp.h"
#include "files.cc"

static inline u32 NextPO2(u32 v) {
	v--;	// handle the zero case
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

/*
==============================================================

MEMORY MANAGEMENT

==============================================================
*/


//
// stack based allocator
//
void *GetMemStackPos(MemoryStack *ms) {
	void *ptr = ms->base_ptr + ms->bytes_used;
	return ptr;
}

// NOTE: dont use the _Push_ and _Pop_ functions directly, go through the macros
// FIXME: pass alignment
void *_Push_(MemoryStack *ms, size_t num_bytes) {
	CheckMemory((ms->bytes_used + num_bytes) <= ms->max_size);
	void *ptr = ms->base_ptr + ms->bytes_used;
	ms->bytes_used += num_bytes;

	return ptr;
}

void _Pop_(MemoryStack *ms, size_t num_bytes) {
	CheckMemory(((int)ms->bytes_used - (int)num_bytes) >= 0);
	memset(ms->base_ptr + ms->bytes_used - num_bytes, 0, num_bytes);
	ms->bytes_used -= num_bytes;
}

// FIXME: pass flags for protect, reserving etc...
#if 0
static MemoryStack InitStackMemory(size_t num_bytes) {
	MemoryStack ms = {};
	void *ptr = VirtualAlloc(0, num_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	ms.base_ptr = (byte *)ptr;
	ms.max_size = num_bytes;
	ms.bytes_used = 0;

	if (!ms.base_ptr) {
		Sys_Print("Failed to init the stack allocator\n");
		Com_Quit();
	}

	return ms;
}
#endif

/*
==============================================================

COMMON

==============================================================
*/

#if 0
#define MapAsciiToTTF(c) (c) - 65
#else
inline static int MapLowerAsciiToTTF(char c) {
	int result = c - 97;
	Assert(result >= 0 && result <= 25);

	return result;
}
inline static int MapHigherAsciiToTTF(char c) {
	int result = c - 65;
	Assert(result >= 0 && result <= 25);

	return result + 26;
}
#endif

static r32 global_game_time_residual;
static int global_game_frame;

static void *global_virtual_memory;
void Com_Allocate(Platform **pf, Renderer **ren) {
	// make sure we are allocating memory for the first time
	Assert(!*pf);
	Assert(!*ren);

	// start allocating
	void *ptr = VirtualAlloc(0, MAX_PERM_MEMORY + MAX_TEMP_MEMORY + sizeof(Platform), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   memset(ptr, 0, MAX_PERM_MEMORY + MAX_TEMP_MEMORY + sizeof(Platform));
   global_virtual_memory = ptr;

	*pf = (Platform *)ptr;
	// FIXME: atm the base pointers of permanent and temp stacks may overlap if permanent allocations are made carelessly 
	(*pf)->main_memory_stack.perm_data.base_ptr = (byte *)(*pf) + sizeof(Platform);
	(*pf)->main_memory_stack.perm_data.max_size = MAX_PERM_MEMORY;
	(*pf)->main_memory_stack.perm_data.bytes_used = 0;

	(*pf)->main_memory_stack.temp_data.base_ptr = (*pf)->main_memory_stack.perm_data.base_ptr + MAX_PERM_MEMORY;
	(*pf)->main_memory_stack.temp_data.max_size = MAX_TEMP_MEMORY;
	(*pf)->main_memory_stack.temp_data.bytes_used = 0;

	(*pf)->game_state = PushStruct(&(*pf)->main_memory_stack.perm_data, GameState);
	(*pf)->input_state = PushStruct(&(*pf)->main_memory_stack.perm_data, Input);

	// renderer
	//Assert(MAX_NUM_POLYS < 0xffff);
	//Assert(MAX_RENDER_BUFFER < MEGABYTES(10));
	*ren = PushStruct(&(*pf)->main_memory_stack.perm_data, Renderer);
	(*ren)->back_end.polys = PushArray(&(*pf)->main_memory_stack.perm_data, MAX_NUM_POLYS, Poly);
	(*ren)->back_end.poly_verts = PushArray(&(*pf)->main_memory_stack.perm_data, MAX_NUM_POLY_VERTS, PolyVert);
   (*ren)->back_end.vis_polys = PushArray(&(*pf)->main_memory_stack.perm_data, MAX_NUM_POLYS*2, Poly);
	(*ren)->back_end.vis_poly_verts = PushArray(&(*pf)->main_memory_stack.perm_data, MAX_NUM_POLY_VERTS*2, PolyVert);
	(*ren)->back_end.lights = PushArray(&(*pf)->main_memory_stack.perm_data, MAX_NUM_LIGHTS, Light);
	(*ren)->back_end.cmds.buffer_base = PushSize(&(*pf)->main_memory_stack.perm_data, MAX_RENDER_BUFFER, byte);
	(*ren)->back_end.cmds.max_buffer_size = MAX_RENDER_BUFFER;
	(*ren)->back_end.cmds.used_buffer_size = 0;
	//(*ren)->back_end.entities = (*pf)->game_state->entities;
}

void Com_SetupIO(Platform **pf) {
	Assert(*pf);
	(*pf)->file_ptrs.free_file = DebugFreeFile;
	(*pf)->file_ptrs.read_file = DebugReadFile;
	(*pf)->file_ptrs.write_file = DebugWriteFile;
}

void Com_LoadEntities(Platform *pf) {
	InitEntities(pf);
}

void Com_LoadTextures(Platform *pf, Renderer *r) {
	FileInfo fi = pf->file_ptrs.read_file("check2.bmp");

   int w, h;
   Bitmap test_texture;
   byte *bmp_data = bmp_load(fi.data, fi.size, 0, &w, &h);
   Assert(bmp_data);
   test_texture.data = bmp_data;
   test_texture.dim.v.x = w;
   test_texture.dim.v.y = h;

   r->back_end.test_texture = test_texture;
}

void Com_LoadFonts(Platform *pf, Renderer *r) {
	FileInfo ttf_file = pf->file_ptrs.read_file("C:/Windows/Fonts/tahoma.ttf");

   for (int i = 'a'; i <= 'z'; ++i) {
		pf->game_state->test_font[MapLowerAsciiToTTF((char)i)] = TTF_LoadGlyph(&pf->main_memory_stack.temp_data, &ttf_file, i);
	}
	for (int i = 'A'; i <= 'Z'; ++i) {
		pf->game_state->test_font[MapHigherAsciiToTTF((char)i)] = TTF_LoadGlyph(&pf->main_memory_stack.temp_data, &ttf_file, i);
	}
   pf->file_ptrs.free_file(&ttf_file);
}

static void FlushPolys(RendererBackend *rb) {
	int num_polys = rb->num_polys;
   int num_poly_verts = rb->num_poly_verts;
	int vis_num_polys = rb->vis_num_polys;
   int vis_num_poly_verts = rb->vis_num_poly_verts;
   memset(rb->polys, 0, num_polys*sizeof(*rb->polys));
   memset(rb->poly_verts, 0, num_poly_verts*sizeof(*rb->poly_verts));
   memset(rb->vis_polys, 0, vis_num_polys*sizeof(*rb->vis_polys));
   memset(rb->vis_poly_verts, 0, vis_num_poly_verts*sizeof(*rb->vis_poly_verts));
	rb->num_polys = 0;
	rb->num_poly_verts = 0;
	rb->vis_num_polys = 0;
	rb->vis_num_poly_verts = 0;
}

static void ProcessEvent(SysEvent se) {
}

static int Com_ModifyFrameMsec(int frame_msec) {
	int clamped_msec = (int)(MSEC_PER_SIM + MSEC_PER_SIM);
	if (frame_msec > clamped_msec) {
		frame_msec = clamped_msec;
	}

	return frame_msec;
}

static void Com_RunEventLoop() {
	SysEvent se;

	for (;;) {
		se = Com_GetEvent();

		// if no more events bail out
		if (se.type == SET_NONE) {
			return;
		}

		ProcessEvent(se);
	}
}

void Com_RunFrame(Platform *pf, Renderer *ren) {
	Sys_GenerateEvents();
	IN_GetInput(pf->input_state);
	IN_HandleInput(pf->input_state);

	Com_RunEventLoop();

   static bool first_run = true;
	int num_frames_to_run = 0;

	for (;;) {
		const int current_frame_time = Sys_GetMilliseconds();
		static int last_frame_time = current_frame_time;	
		int delta_milli_seconds = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		delta_milli_seconds = Com_ModifyFrameMsec(delta_milli_seconds);

		global_game_time_residual += delta_milli_seconds;

		for (;;) {
			// how much to wait before running the next frame
			if (global_game_time_residual < MSEC_PER_SIM) {		
				break;
			}
			global_game_time_residual -= MSEC_PER_SIM;
			global_game_frame++;
			num_frames_to_run++;
		}

		if (num_frames_to_run > 0) {
			// run the frames
			break;
		}
		Sys_Sleep(0);
	}

	if (first_run) {
      RF_SetupProjection(&ren->front_end.current_view);
      first_run = false;
	}
   RF_UpdateView(&ren->front_end.current_view);
	R_BeginFrame(&ren->back_end.target, &ren->back_end.cmds);
	UpdateEntities(pf->game_state, ren, pf->input_state, MSEC_PER_SIM / 1000.0f, num_frames_to_run);
	RenderEntities(pf->game_state, ren);


#if 1
	// font testing
	{
		r32 text_pos = (r32)ren->back_end.target.height;
		const r32 line_gap = 20.0f;
		R_PushTextCmd(&ren->back_end.cmds, "wasd to move", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press enter to center the camera", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press x y z to pitch yaw and roll the cube", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Hold shift and press x y z to reverse pitch yaw and roll the cube", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press c to toggle console", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press esc to exit", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
	}
#endif

	{
		int last_render_time = Sys_GetMilliseconds();
		R_EndFrame(&ren->back_end.target, &ren->back_end.cmds);
      FlushPolys(&ren->back_end);
		int	now_render_time = Sys_GetMilliseconds();
		int	render_frame_msec = now_render_time - last_render_time;

		char buffer[64];
		sprintf_s(buffer, "Render time: %d ms\n", render_frame_msec);
		OutputDebugStringA(buffer);
	}
}

void Com_Quit() {
   VirtualFree(global_virtual_memory, 0, MEM_RELEASE);
	Sys_Quit();
}

//
// event queue (ring buffer)
//
#define	MAX_COM_QUED_EVENTS		(1 << 8)
#define	MASK_COM_QUED_EVENTS	(MAX_COM_QUED_EVENTS - 1)

static SysEvent global_com_event_queue[MAX_COM_QUED_EVENTS];
static u8 global_com_event_head, global_com_even_tail;

#if 0
void Com_GetEvent(int time, SysEventType ev_type, int value, int value2, int data_len, void *data) {
	SysEvent *ev = &global_sys_event_queue[global_sys_event_head & MASK_SYS_QUED_EVENTS];

	if (global_sys_event_head - global_sys_event_tail >= MAX_SYS_QUED_EVENTS) {
		EventOverflow;
		global_sys_event_tail++;
	}

	global_sys_event_head++;

	time = (time == 0) ? Sys_GetMilliseconds() : time;

	ev->time = time;
	ev->type = ev_type;
	ev->value = value;
	ev->value2 = value2;
	ev->data_size = data_len;
	ev->data = data;
}
#endif

SysEvent Com_GetEvent() {
	if (global_com_event_head > global_com_even_tail) {
		global_com_even_tail++;
		return global_com_event_queue[(global_com_even_tail - 1) & MASK_COM_QUED_EVENTS];
	}

	return Sys_GetEvent();
}

u32 PackRGBA(Vec4 color) {
	u32 result = (RoundReal32ToU32(color.c.a * 255.0f) << 24 |
				  RoundReal32ToU32(color.c.r * 255.0f) << 16 |
				  RoundReal32ToU32(color.c.g * 255.0f) << 8  |
				  RoundReal32ToU32(color.c.b * 255.0f));

	return result;
}

u32 PackRGBA(r32 r, r32 g, r32 b, r32 a) {
	Vec4 v = {};
	v.c.r = r;
	v.c.g = g;
	v.c.b = b;
	v.c.a = a;

	u32 result = PackRGBA(v);

	return result;
}

Vec4 UnpackRGBA(u32 color) {
	Vec4 result = {};

	result.c.a = (r32)(((color & 0xff000000) >> 24u) / 255.0f);
	result.c.r = (r32)(((color & 0xff0000) >> 16u) / 255.0f);
	result.c.g = (r32)(((color & 0xff00) >> 8u) / 255.0f);
	result.c.b = (r32)((color & 0xff >> 0u) / 255.0f);

	return result;
}


/*
==============================================================

String handling

==============================================================
*/

int StrCmp(const char* a, const char* b) {
	Assert(a && b);
	do {
		if(*a < *b) {
			return -1;
		} else if(*a > *b) {
			return 1;
		}
	} while(*a++ && *b++);

	return 0;
}

void StrCopyLen(char *dst, const char* src, size_t len) {
	Assert(dst && src);
	while ((*dst++ = *src++) && --len > 0) 
		;
}

Bitmap MakeBitmap(MemoryStack *ms, int width, int height) {
	Bitmap result = {};
	result.dim[0] = width;
	result.dim[1] = height;
	result.data = PushArray(ms, BYTES_PER_PIXEL * width * height, byte);

	return result;
}








