#include "common.h"
#include "renderer.h"
#include "r_cmds.h"
#include "plg_loader.h"
#include "lights.h"

#include "files.cc"

/*
==============================================================

MEMORY MANAGEMENT

==============================================================
*/

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

static r32 yaw = 0.0f;
void Com_Allocate(Platform **pf, Renderer **ren) {
	// make sure we are allocating memory for the first time
	Assert(!*pf);
	Assert(!*ren);

	// start allocating
	void *ptr = VirtualAlloc(0, MAX_PERM_MEMORY + MAX_TEMP_MEMORY + sizeof(Platform), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   memset(ptr, 0, MAX_PERM_MEMORY+MAX_TEMP_MEMORY+sizeof(Platform));

	// platform
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
	(*ren)->back_end.lights = PushArray(&(*pf)->main_memory_stack.perm_data, MAX_NUM_LIGHTS, Light);
	(*ren)->back_end.cmds.buffer_base = PushSize(&(*pf)->main_memory_stack.perm_data, MAX_RENDER_BUFFER, byte);
	(*ren)->back_end.cmds.max_buffer_size = MAX_RENDER_BUFFER;
	(*ren)->back_end.cmds.used_buffer_size = 0;
	//(*ren)->back_end.entities = (*pf)->game_state->entities;
}

void Com_Init(Platform **pf) {
	Assert(*pf);
	(*pf)->file_ptrs.free_file = DebugFreeFile;
	(*pf)->file_ptrs.read_file = DebugReadFile;
	(*pf)->file_ptrs.write_file = DebugWriteFile;

	FileInfo ttf_file = (*pf)->file_ptrs.read_file("C:/Windows/Fonts/cambriai.ttf");

	for (int i = 'a'; i <= 'z'; ++i) {
		(*pf)->game_state->test_font[MapLowerAsciiToTTF((char)i)] = TTF_Init(&(*pf)->main_memory_stack.temp_data, &ttf_file, i);
	}
	for (int i = 'A'; i <= 'Z'; ++i) {
		(*pf)->game_state->test_font[MapHigherAsciiToTTF((char)i)] = TTF_Init(&(*pf)->main_memory_stack.temp_data, &ttf_file, i);
	}
	(*pf)->file_ptrs.free_file(&ttf_file);
}

void Com_LoadEntities(Platform *pf) {

	// FIXME: testing entity stuff!!
	size_t max_entity_memory = 1024 << 5;
	InitEntities(pf, max_entity_memory);
	//BaseEntity common_ent = {};

	//common_ent.type = Cube;
	//FileInfo cube_assets = pf->file_ptrs.read_file("cube1.plg");

	// FIXME: testing!!
	// FIXME: 0 hardcoded for player for now
	//Assert(PLG_LoadMesh(&common_ent, cube_assets.data, cube_assets.size));
	//memcpy(&pf->game_state->entities[0], &common_ent, sizeof(Entity));
	//pf->game_state->entities[0].type_enum = EntityType_player;

	//for (int i = 1; i < num_entities; ++i) {
	//	Vec3 world_pos = {-100.0f + (20.0f * i), 0.0f, 200.0f};
	//	memcpy(&pf->game_state->entities[i], &common_ent, sizeof(Entity));
	//	pf->game_state->entities[i].status.world_pos = world_pos;
	//	pf->game_state->entities[i].type_enum = EntityType_cube;
	//}

	//pf->file_ptrs.free_file(&cube_assets);
}

static void ClearRenderState(RendererBackend *rb) {
	int num_polys = rb->num_polys;
	for (int j = 0; j < num_polys; ++j) {
		rb->polys[j].state &= (~POLY_STATE_BACKFACE);
		rb->polys[j].state &= (~POLY_STATE_LIT);
		rb->polys[j].state &= (~POLY_STATE_CULLED);
	}
	rb->num_polys = 0;
	rb->num_poly_verts = 0;
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

#if 0
static void Com_SimFrame(r32 dt, r32 dt_residual, int num_frames, int num_entities, Entity *ents, Input *in, ViewSystem *current_view) {
	// test stuff
	static r32 rot_mat_y[3][3];
	static r32 sim_dt = 0.0f;
	sim_dt += dt;
	// FIXME: add matrix returning routines
	rot_mat_y[1][0] = 0.0f;
	rot_mat_y[1][1] = 1.0f;
	rot_mat_y[1][2] = 0.0f;

	r32 speed = 30.0f;
	// FIXME: add matrix returning routines
	r32 rot_mat_x[3][3];
	rot_mat_x[0][0] = 1.0f;
	rot_mat_x[0][1] = 0.0f;
	rot_mat_x[0][2] = 0.0f;

	r32 rot_mat_z[3][3];
	rot_mat_z[2][0] = 0.0f;
	rot_mat_z[2][1] = 0.0f;
	rot_mat_z[2][2] = 1.0f;

	// FIXME: just for testing!!!!!!!!
	r32 rot_theta = DEG2RAD(-1.0f*0.1f);
	rot_mat_x[1][0] = 0.0f;
	rot_mat_x[1][1] = cos(rot_theta);
	rot_mat_x[1][2] = sin(rot_theta);

	rot_mat_x[2][0] = 0.0f;
	rot_mat_x[2][1] = -rot_mat_x[1][2];
	rot_mat_x[2][2] = rot_mat_x[1][1];

	rot_mat_z[0][0] = cos(rot_theta);
	rot_mat_z[0][1] = sin(rot_theta);
	rot_mat_z[0][2] = 0.0f;

	rot_mat_z[1][0] = -rot_mat_z[0][1];
	rot_mat_z[1][1] = rot_mat_z[0][0];
	rot_mat_z[1][2] = 0.0f;

	for (int i = 0; i < num_frames; ++i) {
		for (int j = 0; j < num_entities; ++j) {
			if (auto player = GetAnonType(&ents[j], player, EntityType_)) {
				PolyVert *verts = player->local_vertex_array;
				int num_local_verts = ArrayCount(player->local_vertex_array);
				RotatePoints(rot_mat_z, verts, num_local_verts); 
				RotatePoints(rot_mat_x, verts, num_local_verts); 
				Vec3 acc = {};
				if (in->keys['W'].down) {
					acc = Vec3Norm(current_view->world_orientation.dir);
					acc = acc * 1.0f;
				}
				if (in->keys['A'].down) {
					// FIXME: add matrix returning routines
					rot_mat_y[0][0] = cos(DEG2RAD(3.0f));
					rot_mat_y[0][2] = sin(DEG2RAD(3.0f));

					rot_mat_y[2][0] = -rot_mat_y[0][2];
					rot_mat_y[2][2] = rot_mat_y[0][0];

					if (sim_dt > (1.0f / 60.0f)) {
						yaw -= 3.0f;
						current_view->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
						current_view->world_orientation.dir[2] = cosf(DEG2RAD(yaw));
						for (int i = 0; i < ents[0].status.num_verts; ++i) {
							Mat1x3Mul(&verts[i].xyz, &verts[i].xyz, rot_mat_y);
						}
					}
				}
				if (in->keys['S'].down) {
					acc = Vec3Norm(current_view->world_orientation.dir);
					acc = acc * (-1.0f);
				}
				if (in->keys['D'].down) {
					// FIXME: add matrix returning routines
					rot_mat_y[0][0] = cos(DEG2RAD(3.0f));
					rot_mat_y[0][2] = -sin(DEG2RAD(3.0f));

					rot_mat_y[2][0] = -rot_mat_y[0][2];
					rot_mat_y[2][2] = rot_mat_y[0][0];

					if (sim_dt > (1.0f / 60.0f)) {
						yaw += 3.0f;
						current_view->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
						current_view->world_orientation.dir[2] = cosf(DEG2RAD(yaw));
						for (int i = 0; i < ents[0].status.num_verts; ++i) {
							Mat1x3Mul(&verts[i].xyz, &verts[i].xyz, rot_mat_y);
						}
					}
				}
				if (in->keys[ENTER_KEY].released) {
					Vec3Init(current_view->world_orientation.dir, 0.0f, 0.0f, 1.0f);
					Vec3Init(current_view->world_orientation.origin, 0.0f, 0.0f, 0.0f);
					
					rot_mat_y[0][0] = cos(DEG2RAD(yaw));
					rot_mat_y[0][2] = sin(DEG2RAD(yaw));

					rot_mat_y[2][0] = -rot_mat_y[0][2];
					rot_mat_y[2][2] = rot_mat_y[0][0];

					// reset player yaw angle and position
					if (yaw != 0.0f) {
						for (int i = 0; i < ents[0].status.num_verts; ++i) {
							Mat1x3Mul(&verts[i].xyz, &verts[i].xyz, rot_mat_y);
						}
					}

					yaw = 0.0f;
				}

				acc = Vec3Norm(acc);
				acc = acc * speed;
				acc = acc + (current_view->velocity * -0.95f);	// hack!!!
				current_view->world_orientation.origin = (acc * 0.5f * Square(dt)) + (current_view->velocity * dt) + current_view->world_orientation.origin;
				current_view->velocity = acc * dt + current_view->velocity;
			} 
		}
	}
	if (sim_dt > (1.0f / 60.0f)) {
		sim_dt -= (1.0f / 60.0f);
	}
}
#endif

#if 1
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

	R_PushPolysCmd(&ren->back_end.cmds,
				  ren->back_end.polys,
				  ren->back_end.poly_verts,
				  ren->back_end.num_polys,
				  ren->front_end.is_wireframe);

	// font testing
	{
		r32 text_pos = (r32)ren->back_end.target.height;
		r32 line_gap = 20.0f;
		R_PushTextCmd(&ren->back_end.cmds, "WASD to move", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press space to toggle wireframe mode", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press enter to center the player", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press l to toggle ambient lighting", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press c to toggle console", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
		R_PushTextCmd(&ren->back_end.cmds, "Press esc to exit", pf->game_state->test_font, MV2(10.0f, text_pos -= line_gap));
	}

	// frame timing
	{
		static int last_time = Sys_GetMilliseconds();
		int	now_time = Sys_GetMilliseconds();
		int	frame_msec = now_time - last_time;
		last_time = now_time;

		R_EndFrame(&ren->back_end.target, &ren->back_end.cmds);

		ClearRenderState(&ren->back_end);

		char buffer[64];
		//sprintf_s(buffer, "Frame msec %d\n", frame_msec);
		//OutputDebugStringA(buffer);
	}
}
#endif

void Com_Quit() {
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








