#include "common.h"
#include "r_cmds.h"
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

static r32 global_game_time_residual;
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
	pf.main_memory_stack.perm_data = InitStackMemory(MAX_PERM_MEMORY);
	pf.main_memory_stack.temp_data = InitStackMemory(MAX_TEMP_MEMORY);
	pf.game_state = PushStruct(pf.main_memory_stack.perm_data, GameState);

	//IN_ClearKeyStates(pf.input);

	// just for prototyping purposes
	Entity *player_ent = PushStruct(pf.main_memory_stack.perm_data, Entity);
	player_ent->mesh = PushStruct(pf.main_memory_stack.perm_data, Mesh);
	player_ent->mesh->local_verts = PushStruct(pf.main_memory_stack.perm_data, VertexGroup);
	player_ent->mesh->trans_verts = PushStruct(pf.main_memory_stack.perm_data, VertexGroup);
	player_ent->mesh->polys = PushStruct(pf.main_memory_stack.perm_data, PolyGroup);

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
	PLG_LoadMesh(player_ent, &fp, world_pos);
	memcpy(&pf.game_state->entities[pf.game_state->num_entities++], player_ent, sizeof(Entity));

	const int num_entities = 5;
	for (int i = 0; i < num_entities; ++i) {
		// just for prototyping purposes
		Entity *ent = PushStruct(pf.main_memory_stack.perm_data, Entity);
		ent->mesh = PushStruct(pf.main_memory_stack.perm_data, Mesh);
		ent->mesh->local_verts = PushStruct(pf.main_memory_stack.perm_data, VertexGroup);
		ent->mesh->trans_verts = PushStruct(pf.main_memory_stack.perm_data, VertexGroup);
		ent->mesh->polys = PushStruct(pf.main_memory_stack.perm_data, PolyGroup);


		// FIXME: move elsewhere
		// position the objects randomly
		Vec3 world_pos = {-100.0f + (i * 50.0f), 20.0f, 200.0f};
		PLG_LoadMesh(ent, &fp, world_pos);

		memcpy(&pf.game_state->entities[pf.game_state->num_entities++], ent, sizeof(Entity));
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

	// FIXME: handles only up, down, left, right, space and pause keys
	if (se.type == SET_KEY) {
		int mapped_key = IN_MapKeyToIndex(in, se.value, se.value2, se.time);

		// valid key
		// FIXME: make into switch
		if (mapped_key > ControllerIndexEnum::KEY_INVALID) {
			if ((mapped_key == ControllerIndexEnum::KEY_PAUSE) && !IN_IsKeyDown(in, mapped_key)) {
				paused = !paused;
			} else if ((mapped_key == ControllerIndexEnum::KEY_SPACE) && !IN_IsKeyDown(in, mapped_key)) {
				wireframe = !wireframe;
			} else if ((mapped_key == ControllerIndexEnum::KEY_UP) && IN_IsKeyDown(in, mapped_key)) {
			} else if ((mapped_key == ControllerIndexEnum::KEY_DOWN) && IN_IsKeyDown(in, mapped_key)) {
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

static void Com_SimFrame(r32 dt, r32 dt_residual, int num_frames, Entity *ents, Input *in) {
	for (int i = 0; i < num_frames; ++i) {
	}
}

void Com_RunFrame(Platform *pf, RenderingSystem *rs) {
	Sys_GenerateEvents();
	Com_RunEventLoop(pf->game_state, &pf->input_state);

	// test stuff
	if (paused) {
		return;
	}

	// test stuff
	static b32 first_run = true;

	int num_game_frames_to_run = 0;

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
		}

		if (num_game_frames_to_run > 0) {
			// ready to actually run the frames
			break;
		}
		Sys_Sleep(0);
	}

	Entity *entities = pf->game_state->entities;
	Com_SimFrame(MSEC_PER_SIM, global_game_time_residual, num_game_frames_to_run, entities, &pf->input_state);
	

	// FIXME: will be moved
	// render
	ViewSystem *current_view = &rs->front_end.current_view;
	RendererBackend *rbe = &rs->back_end;
	RenderCommands *cmds = &rs->back_end.cmds;
	if (reset) {
		Vec3Init(current_view->world_orientation.dir, 0.0f, 0.0f, 1.0f);
		Vec3Init(current_view->world_orientation.origin, 0.0f, 0.0f, 0.0f);
		
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

	// FIXME: will be moved
	// movement testing
	if (forward) {
		current_view->world_orientation.origin = 
			current_view->world_orientation.origin + (current_view->world_orientation.dir * 10.0f);
	}

	if (backward) {
		current_view->world_orientation.origin = 
			current_view->world_orientation.origin - (current_view->world_orientation.dir * 10.0f);
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

		current_view->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
		current_view->world_orientation.dir[2] = cosf(DEG2RAD(yaw));

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

		current_view->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
		current_view->world_orientation.dir[2] = cosf(DEG2RAD(yaw));

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
	// FIXME: extract this into a function
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

	// FIXME: these will go through the backend
	R_RenderView(current_view);

	// FIXME: 0 index hardcoded for player in the entities array
	// hacky player third person cam test stuff
	entities[0].status.world_pos = 
		current_view->world_orientation.origin + (current_view->world_orientation.dir * 60.0f);
	entities[0].status.world_pos[1] -= 20.0f;

#if 1
	R_BeginFrame(rbe->vid_sys, cmds);
	// FIXME: 0 index hardcoded for player in the entities array for now
	// FIXME: will be move elsewhere
	for (int i = 0; i < num_entities; ++i) {
		Entity *e = &entities[i];
		// FIXME: reduce the indirection overhead
		Vec3 *verts = e->mesh->local_verts->vert_array;
		Vec3 *trans_verts = e->mesh->trans_verts->vert_array;

		if (i) {
			R_RotatePoints(rot_mat_z, verts, e->status.num_verts); 
			R_RotatePoints(rot_mat_x, verts, e->status.num_verts); 
		}

		R_TransformModelToWorld(e); 

		if (i != 0) {
			e->status.state = R_CullPointAndRadius(current_view, e->status.world_pos);			
		}
		if (!(e->status.state & FCS_CULL_OUT)) {
			R_TransformWorldToView(current_view, e);
			R_AddPolys(rbe, trans_verts, e->mesh->polys->poly_array,
					   ArrayCount(e->mesh->polys->poly_array->vert_indices), e->status.num_polys);
			R_CullBackFaces(current_view, rbe->polys, rbe->poly_verts, rbe->num_polys);
		}
	}
#endif
	R_TransformViewToClip(current_view, rbe->poly_verts, rbe->num_verts);
	R_TransformClipToScreen(current_view, rbe->poly_verts, rbe->num_verts);
	R_DrawMesh(rbe->vid_sys, cmds, rbe->polys, rbe->poly_verts, rbe->num_polys, false);

	R_EndFrame(rbe->vid_sys, cmds);
	// FIXME: move these into ending routine
	rbe->num_polys = 0;
	rbe->num_verts = 0;
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
	int clamped_msec = (int)(MSEC_PER_SIM + MSEC_PER_SIM);
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



