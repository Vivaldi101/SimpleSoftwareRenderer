#include "common.h"
#include "render_queue.h"
#include "plg_loader.h"


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


#if 0
//
// fixed size allocator
//

#define BYTE_INDEX_16	1		// index of first 16-byte payload
#define BYTE_INDEX_32	18		// index of first 16-byte payload
#define BYTE_INDEX_64	51		// index of first 16-byte payload
#define BYTE_INDEX_128	116		// index of first 16-byte payload
#define BYTE_INDEX_256	245		// index of first 16-byte payload
#define BYTE_INDEX_512	502		// index of first 16-byte payload
#define BYTE_INDEX_1024	1015	// index of first 16-byte payload
#define BYTE_INDEX_2048	2040	// index of first 16-byte payload

#define BLOCK_STATE(data, index)	((data)[(index) - 1])	// check if empty block

// ids for blocks
#define FREE_BLOCK 0xfb
#define ALLOCATED_BLOCK 0xab

#define LIST_ROW_SIZE	4088	// size of a row of entries in table


static void InitColumn(FBAllocator *fba, int index) {
	int num_rows = fba->num_rows;
	for (int i = 0; i < num_rows; ++i) {
		BLOCK_STATE(fba->data, index) = FREE_BLOCK;
		index += LIST_ROW_SIZE;
	}
}

static void DestroyFixedBlockAllocator(FBAllocator *fba) {
	fba->num_rows = 0;
	fba->max_size = 0;
	VirtualFree(fba->data, 0, MEM_RELEASE);
}

static int SearchColumn(FBAllocator *fba, int index) {
	int num_rows = fba->num_rows;

	for (int i = 0; i < num_rows; ++i) {
		if (BLOCK_STATE(fba->data, index) == FREE_BLOCK) {
			BLOCK_STATE(fba->data, index) = ALLOCATED_BLOCK;

			return index;
		}
		index += LIST_ROW_SIZE;
	}

	return 0;
}

void *Allocate(FBAllocator *la, size_t num_bytes) {
	int index;

	if (!la->data) {
		Sys_Print("FixedBlock memory system not initialized\n");
		Sys_Quit();
	}

	// FIXME: check for alignment

	if (num_bytes <= 16) {
		if (index = SearchColumn(la, BYTE_INDEX_16)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 32) {
		if (index = SearchColumn(la, BYTE_INDEX_32)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 64) {
		if (index = SearchColumn(la, BYTE_INDEX_64)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 128) {
		if (index = SearchColumn(la, BYTE_INDEX_128)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 256) {
		if (index = SearchColumn(la, BYTE_INDEX_256)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 512) {
		if (index = SearchColumn(la, BYTE_INDEX_512)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 1024) {
		if (index = SearchColumn(la, BYTE_INDEX_1024)) {
			return &la->data[index];
		}
	} else if (num_bytes <= 2048) {
		if (index = SearchColumn(la, BYTE_INDEX_2048)) {
			return &la->data[index];
		}
	}

	// FIXME: handle this?
	Sys_Print("Couldn't allocate memory\n");
	return 0;
}

void Free(FBAllocator *la, void **ptr) {
	int index = (int)((byte *)(*ptr) - la->data);
	index = index % LIST_ROW_SIZE;
	*ptr = 0;

	if (index >= 1 || index < la->max_size) {
		BLOCK_STATE(la->data, index) = FREE_BLOCK;
		switch (index) {
			case BYTE_INDEX_16: {
				memset(&la->data[index], 0, 16); 
			} break; 
			case BYTE_INDEX_32: {
				memset(&la->data[index], 0, 32); 
			} break; 
			case BYTE_INDEX_64: {
				memset(&la->data[index], 0, 64); 
			} break; 
			case BYTE_INDEX_128: {
				memset(&la->data[index], 0, 128); 
			} break; 
			case BYTE_INDEX_256: {
				memset(&la->data[index], 0, 256); 
			} break; 
			case BYTE_INDEX_512: {
				memset(&la->data[index], 0, 512); 
			} break; 
			case BYTE_INDEX_1024: {
				memset(&la->data[index], 0, 1024); 
			} break; 
			case BYTE_INDEX_2048: {
				memset(&la->data[index], 0, 2048); 
			} break; 
		}
	}
}

static byte *InitFixedBlockMemory(size_t num_bytes) {
	FBAllocator *la = (FBAllocator *)VirtualAlloc(0, num_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!la) {
		Sys_Print("Failed to init the fixed sized allocator\n");
		Sys_Quit();
	}

	la->max_size = num_bytes;
	la->num_rows = (int)num_bytes / LIST_ROW_SIZE;
	la->data = (byte *)la + sizeof(FBAllocator);

	InitColumn(la, BYTE_INDEX_16);
	InitColumn(la, BYTE_INDEX_32);
	InitColumn(la, BYTE_INDEX_64);
	InitColumn(la, BYTE_INDEX_128);
	InitColumn(la, BYTE_INDEX_256);
	InitColumn(la, BYTE_INDEX_512);
	InitColumn(la, BYTE_INDEX_1024);
	InitColumn(la, BYTE_INDEX_2048);

	return (byte *)la;
}
#endif

//
// stack based allocator
//

// NOTE: dont use the _Push_ and _Pop_ functions directly, go through the macros
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

static MemoryStack *InitStackMemory(size_t num_bytes) {
	void *base = VirtualAlloc(0, num_bytes + sizeof(MemoryStack), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	MemoryStack *ms = (MemoryStack *)base;
	ms->base_ptr = (byte *)(ms + 1);
	ms->max_size = num_bytes;
	ms->bytes_used = 0;

	if (!ms) {
		Sys_Print("Failed to init the stack allocator\n");
		Sys_Quit();
	}

	return ms;
}

/*
==============================================================

COMMON

==============================================================
*/

static int global_game_time_residual;
static int global_game_frame;

// just for prototyping purposes, will get removed
static r32 yaw = 0.0f;
static b32 forward;
static b32 backward;

static b32 turn_left;
static b32 turn_right;

static Vec3 rot_mat_y[3];

static b32 paused;
static b32 reset;
static b32 wireframe;

Platform Com_Init(void *hinstance, void *wndproc) {
	Platform pf = {};

	Sys_Init();

	// FIXME: macros for memory sizes
	//pf.fb_allocator.data = InitFixedBlockMemory(1024 * LIST_ROW_SIZE);
	// FIXME: make a single double ended stack, with temp allocations coming from the other side
	pf.stack_allocator.perm_data = InitStackMemory(MAX_PERM_MEMORY);
	pf.stack_allocator.temp_data = InitStackMemory(MAX_TEMP_MEMORY);
	pf.input = PushStruct(pf.stack_allocator.perm_data, Input);
	pf.game_state = PushStruct(pf.stack_allocator.perm_data, GameState);

	IN_ClearKeyStates(pf.input);

	// just for prototyping purposes
	Entity *player_mo = PushStruct(pf.stack_allocator.perm_data, Entity);
	player_mo->mesh = PushStruct(pf.stack_allocator.perm_data, Mesh);
	player_mo->mesh->local_verts = PushStruct(pf.stack_allocator.perm_data, VertexGroup);
	player_mo->mesh->trans_verts = PushStruct(pf.stack_allocator.perm_data, VertexGroup);
	player_mo->mesh->polys = PushStruct(pf.stack_allocator.perm_data, PolyGroup);

	// FIXME: move file api elsewhere
	// FIXME: maybe replace the CRT file i/o with win32 api
	FILE *fp;
	fopen_s(&fp, "poly_data.plg", "r");

	if (!fp) {
		Sys_Print("\nCouldn't open file\n");
		Sys_Quit();
	}

	Sys_Print("\nOpening PLG file\n");

	Vec3 world_pos = {};
	PLG_LoadMesh(player_mo, &fp, world_pos);
	memcpy(&pf.game_state->entities[pf.game_state->num_entities++], player_mo, sizeof(Entity));

	const int num_entities = 10;
	for (int i = 0; i < num_entities; ++i) {
		// just for prototyping purposes
		Entity *mo = PushStruct(pf.stack_allocator.perm_data, Entity);
		mo->mesh = PushStruct(pf.stack_allocator.perm_data, Mesh);
		mo->mesh->local_verts = PushStruct(pf.stack_allocator.perm_data, VertexGroup);
		mo->mesh->trans_verts = PushStruct(pf.stack_allocator.perm_data, VertexGroup);
		mo->mesh->polys = PushStruct(pf.stack_allocator.perm_data, PolyGroup);


		// FIXME: move elsewhere
		// position the objects randomly
		Vec3 world_pos = {-100.0f + (i * 50.0f), 20.0f, 200.0f};
		PLG_LoadMesh(mo, &fp, world_pos);

		memcpy(&pf.game_state->entities[pf.game_state->num_entities++], mo, sizeof(Entity));
	}

	if (fp) {
		Sys_Print("\nClosing PLG file\n");
		fclose(fp);
	}


	return pf;
}

static void ProcessEvent(Input *in, SysEvent se) {

	if (se.value == VK_ESCAPE) {
		Sys_Quit();
	}

	// FIXME: remove the globals
	// FIXME: handles only up, down, left, right, space and pause keys
	// FIXME: make into switch
	if (se.type == SET_KEY) {
		int mapped_key = IN_MapKeyToIndex(in, se.value, se.value2, se.time);

		// valid key
		if (mapped_key > ControllerIndexEnum::KEY_INVALID) {
			if ((mapped_key == ControllerIndexEnum::KEY_PAUSE) && !IN_IsKeyDown(in, mapped_key)) {
				paused = !paused;
			} else if ((mapped_key == ControllerIndexEnum::KEY_SPACE) && !IN_IsKeyDown(in, mapped_key)) {
				wireframe = !wireframe;
			}
		}

		if (se.value == VK_RETURN) {
			reset = true;
		}
	}
}

static void Com_RunEventLoop(GameState *gs, Input *in) {
	SysEvent se;

	for (;;) {
		se = Com_GetEvent();

		// if no more events bail out
		if (se.type == SET_NONE) {
			return;
		}

		ProcessEvent(in, se);
	}
}

static void Com_SimFrame(r32 dt, Entity *ents, Input *in) {
}

void Com_RunFrame(Platform *pf, Renderer *ren) {
	Sys_GenerateEvents();
	Com_RunEventLoop(pf->game_state, pf->input);

	// test stuff
	if (paused) {
		return;
	}

	static b32 first_run = true;

	// FIXME: atm this is unused, will get fixed
	int num_game_frames_to_run = 0;

	// test stuff
	r32 rot_mat_x[3][3];
	r32 rot_mat_z[3][3];
	r32 rot_theta = DEG2RAD(-1.0f);
	static r32 view_angle = 0.0f;
	static r32 rot_mat_y[3][3];

	for (;;) {
		const int current_frame_time = Sys_GetMilliseconds();
		static int last_frame_time = current_frame_time;	
		int delta_milli_seconds = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		delta_milli_seconds = Com_ModifyFrameMsec(delta_milli_seconds);

		global_game_time_residual += delta_milli_seconds;

		for (;;) {
			// how much time to wait before running the next frame
			if (global_game_time_residual < MSEC_PER_SIM) {		
				break;
			}
			global_game_time_residual -= MSEC_PER_SIM;
			global_game_frame++;
			num_game_frames_to_run++;
			// if there is enough residual left, we may run additional frames
		}

		if (num_game_frames_to_run > 0) {
			// ready to actually run the frames
			break;
		}
		Sys_Sleep(0);
	}

	Entity *entities = pf->game_state->entities;
	for (int i = 0; i < num_game_frames_to_run; ++i) {
		Com_SimFrame(MSEC_PER_SIM, entities, pf->input);
	}

	// FIXME: will be moved elsewhere
	RenderCommands *rc = ren->commands;

	if (reset) {
		Vec3Init(ren->current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
		Vec3Init(ren->current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);
		
		rot_mat_y[0][0] = cos(DEG2RAD(yaw));
		rot_mat_y[0][2] = sin(DEG2RAD(yaw));

		rot_mat_y[2][0] = -sin(DEG2RAD(yaw));
		rot_mat_y[2][2] = cos(DEG2RAD(yaw));

		// reset player camera
		if (yaw != 0.0f) {
			Vec3 *verts = entities[0].mesh->local_verts->vert_array;
			for (int i = 0; i < entities[0].status.num_verts; ++i) {
				Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
			}
		}

		reset = false;
		yaw = 0.0f;
	}

	// movement testing
	if (forward) {
		ren->current_view.world_orientation.origin = 
			ren->current_view.world_orientation.origin + (ren->current_view.world_orientation.dir * 10.0f);
	}

	if (backward) {
		ren->current_view.world_orientation.origin = 
			ren->current_view.world_orientation.origin - (ren->current_view.world_orientation.dir * 10.0f);
	}

	if (turn_left) {
		yaw -= 1.0f;

		rot_mat_y[0][0] = cos(DEG2RAD(1.0f));
		rot_mat_y[0][1] = 0.0f;
		rot_mat_y[0][2] = sin(DEG2RAD(1.0f));

		rot_mat_y[1][0] = 0.0f;
		rot_mat_y[1][1] = 1.0f;
		rot_mat_y[1][2] = 0.0f;

		rot_mat_y[2][0] = -sin(DEG2RAD(1.0f));
		rot_mat_y[2][1] = 0.0f;
		rot_mat_y[2][2] = cos(DEG2RAD(1.0f));

		ren->current_view.world_orientation.dir[0] = sinf(DEG2RAD(yaw));
		ren->current_view.world_orientation.dir[2] = cosf(DEG2RAD(yaw));

		Vec3 *verts = entities[0].mesh->local_verts->vert_array;
		for (int i = 0; i < entities[0].status.num_verts; ++i) {
			Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
		}
	}

	if (turn_right) {
		yaw += 1.0f;

		rot_mat_y[0][0] = cos(DEG2RAD(1.0f));
		rot_mat_y[0][1] = 0.0f;
		rot_mat_y[0][2] = -sin(DEG2RAD(1.0f));

		rot_mat_y[1][0] = 0.0f;
		rot_mat_y[1][1] = 1.0f;
		rot_mat_y[1][2] = 0.0f;

		rot_mat_y[2][0] = sin(DEG2RAD(1.0f));
		rot_mat_y[2][1] = 0.0f;
		rot_mat_y[2][2] = cos(DEG2RAD(1.0f));

		ren->current_view.world_orientation.dir[0] = sinf(DEG2RAD(yaw));
		ren->current_view.world_orientation.dir[2] = cosf(DEG2RAD(yaw));

		Vec3 *verts = entities[0].mesh->local_verts->vert_array;
		for (int i = 0; i < entities[0].status.num_verts; ++i) {
			Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
		}
	}

	// test stuff
	rot_mat_x[0][0] = 1.0f;
	rot_mat_x[0][1] = 0.0f;
	rot_mat_x[0][2] = 0.0f;

	rot_mat_x[1][0] = 0.0f;
	rot_mat_x[1][1] = cos(rot_theta);
	rot_mat_x[1][2] = sin(rot_theta);

	rot_mat_x[2][0] = 0.0f;
	rot_mat_x[2][1] = -sin(rot_theta);
	rot_mat_x[2][2] = cos(rot_theta);

	rot_mat_z[0][0] = cos(rot_theta);
	rot_mat_z[0][1] = sin(rot_theta);
	rot_mat_z[0][2] = 0.0f;

	rot_mat_z[1][0] = -sin(rot_theta);
	rot_mat_z[1][1] = cos(rot_theta);
	rot_mat_z[1][2] = 0.0f;

	rot_mat_z[2][0] = 0.0f;
	rot_mat_z[2][1] = 0.0f;
	rot_mat_z[2][2] = 1.0f;


	// test stuff
	// FIXME: extract these into a function
	int num_entities = pf->game_state->num_entities;
	for (int i = 0; i < num_entities; ++i) {
		Entity *current_mo = &entities[i];
		int num_polys = current_mo->status.num_polys;

		// clear mesh states
		// FIXME: reduce the indirection overhead
		Poly *polys = current_mo->mesh->polys->poly_array;
		for (int j = 0; j < num_polys; ++j) {
			polys[j].state = polys[j].state & (~POLY_STATE_BACKFACE);
			polys[j].state = polys[j].state & (~FCS_CULL_OUT);
		}
	}

	R_RenderView(ren);

	// FIXME: 0 index hardcoded for player in the entities array
	// acky player third person cam test stuff
	entities[0].status.world_pos = 
		ren->current_view.world_orientation.origin + (ren->current_view.world_orientation.dir * 60.0f);
	entities[0].status.world_pos[1] -= 20.0f;

	// FIXME: 0 index hardcoded for player in the entities array
	// FIXME: will be move elsewhere
	for (int i = 0; i < num_entities; ++i) {
		Entity *current_mo = &entities[i];
		// FIXME: reduce the indirection overhead
		Vec3 *verts = current_mo->mesh->local_verts->vert_array;

		if (i) {
			R_RotatePoints(rot_mat_z, verts, current_mo->status.num_verts); 
			R_RotatePoints(rot_mat_x, verts, current_mo->status.num_verts); 
		}

		R_TransformModelToWorld(ren, current_mo); 

		if (i) {
			current_mo->status.state = R_CullPointAndRadius(ren, current_mo->status.world_pos);			
		}
		if (!(current_mo->status.state & FCS_CULL_OUT)) {
			R_TransformWorldToView(ren, current_mo);
			R_CullBackFaces(ren, current_mo);
			R_TransformViewToClip(ren, current_mo);
			R_TransformClipToScreen(ren, current_mo);
			if (wireframe) {
				R_DrawWireframeMesh(ren, current_mo);
			} else {
				R_DrawSolidMesh(ren, current_mo);
			}
		}
	}

	ApplyRenderCommand(rc, ShowScreen, "");
	EndRenderCommand(rc, ShowScreen);

	ApplyRenderCommand(rc, ClearScreen, "%d", OffsetOf(fill_color, ClearScreen), 0);
	EndRenderCommand(rc, ClearScreen);

	ExecuteRenderCommands(rc, ren);

	{
		static int last_time = Sys_GetMilliseconds();
		int	now_time = Sys_GetMilliseconds();
		int	frame_msec = now_time - last_time;
		last_time = now_time;
		char buffer[64];
		sprintf_s(buffer, "Frame time: %d, Yaw: %f\n", frame_msec, yaw);
		OutputDebugStringA(buffer);
	}

	if (first_run) {
		first_run = false;
	}
}
void Com_Quit() {
	Sys_Quit();
}

//
// event queue
//
#define	MAX_COM_QUED_EVENTS		1024
#define	MASK_COM_QUED_EVENTS	(MAX_COM_QUED_EVENTS - 1)

static SysEvent global_com_event_queue[MAX_COM_QUED_EVENTS];
static u32 global_com_event_head, global_com_even_tail;

SysEvent Com_GetEvent() {
	if (global_com_event_head > global_com_even_tail) {
		global_com_even_tail++;
		return global_com_event_queue[(global_com_even_tail - 1) & MASK_COM_QUED_EVENTS];
	}

	return Sys_GetEvent();
}

static int Com_ModifyFrameMsec(int frame_msec) {
	int clamped_msec = MSEC_PER_SIM + MSEC_PER_SIM;
	if (frame_msec > clamped_msec) {
		frame_msec = clamped_msec;
	}

	return frame_msec;
}

#if 0
void ProcessStruct(X* x, char *format, ...) {
	char *start, *p, *q = (char *)x;
	VaStart(start, format);

	p = format;
	for (; *p; ++p) {
		if (*p == '%') {
			switch (*(p + 1)) {
				case 'd': {
					int offset = VaArg(start, int);
					Assert(offset >= 0);
					Assert(offset < sizeof(*x));
					int data = VaArg(start, int);
					*(int *)(q + offset) = data;
				} break;
				case 'f': {
					int offset = VaArg(start, int);
					Assert(offset >= 0);
					Assert(offset < sizeof(*x));
					double data = VaArg(start, double);
					*(float *)(q + offset) = (float)data;
				} break;

				default: break;
			}
		}
	}
}
#endif



