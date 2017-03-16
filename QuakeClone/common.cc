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

static FBAllocator *InitFixedBlockMemory(size_t num_bytes) {
	FBAllocator *la = (FBAllocator *)VirtualAlloc(0, num_bytes + sizeof(FBAllocator), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

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

	return la;
}

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

	pf.fb_allocator = InitFixedBlockMemory(1024 * LIST_ROW_SIZE);
	// FIXME: make a single double ended stack, with temp allocations coming from the other side
	pf.perm_data = InitStackMemory(MEGABYTES(16));
	pf.temp_data = InitStackMemory(MEGABYTES(64));
	pf.input = PushStruct(pf.perm_data, Input);

	IN_ClearKeyStates(pf.input);

	R_Init(&pf, hinstance, wndproc);

	// just for prototyping purposes
	MeshObject *player_mo = (MeshObject *)Allocate(pf.fb_allocator, sizeof(MeshObject));
	player_mo->mesh = (MeshData *)Allocate(pf.fb_allocator, sizeof(MeshData));
	player_mo->mesh->local_verts = (VertexGroup *)Allocate(pf.fb_allocator, sizeof(VertexGroup));
	player_mo->mesh->trans_verts = (VertexGroup *)Allocate(pf.fb_allocator, sizeof(VertexGroup));
	player_mo->mesh->polys = (PolyGroup *)Allocate(pf.fb_allocator, sizeof(PolyGroup));

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
	PLG_LoadMeshObject(player_mo, &fp, world_pos);
	memcpy(&pf.renderer->back_end->entities[pf.renderer->back_end->num_entities++], player_mo, sizeof(MeshObject));

	const int num_entities = 10;
	for (int i = 0; i < num_entities; ++i) {
		// just for prototyping purposes
		MeshObject *mo = (MeshObject *)Allocate(pf.fb_allocator, sizeof(MeshObject));
		mo->mesh = (MeshData *)Allocate(pf.fb_allocator, sizeof(MeshData));
		mo->mesh->local_verts = (VertexGroup *)Allocate(pf.fb_allocator, sizeof(VertexGroup));
		mo->mesh->trans_verts = (VertexGroup *)Allocate(pf.fb_allocator, sizeof(VertexGroup));
		mo->mesh->polys = (PolyGroup *)Allocate(pf.fb_allocator, sizeof(PolyGroup));


		// FIXME: move elsewhere
		// position the objects randomly
		Vec3 world_pos = {-100.0f + (i * 50.0f), 20.0f, 200.0f};
		PLG_LoadMeshObject(mo, &fp, world_pos);

		memcpy(&pf.renderer->back_end->entities[pf.renderer->back_end->num_entities++], mo, sizeof(MeshObject));
	}

	if (fp) {
		Sys_Print("\nClosing PLG file\n");
		fclose(fp);
	}


	return pf;
}

static void ProcessEvent(Input *in, SysEvent se) {

	// FIXME: move input handling elsewhere
	// ALL of this is just for prototyping purposes
	if (se.value == VK_ESCAPE) {
		//Sys_Print(se.ev_value);
		Sys_Quit();
	}

	if (se.type == SET_KEY) {
		IN_HandleKeyEvent(in, se.value, se.value2, se.time);
	}

#if 1
	if (se.value == VK_PAUSE && !Key_IsDown(in->keys, se.value)) {
		paused = !paused;
	}

	if (se.value == VK_SPACE && !Key_IsDown(in->keys, se.value)) {
		wireframe = !wireframe;
	} 

	if (se.value == VK_UP && Key_IsDown(in->keys, se.value)) {
		forward = true;
	} else {
		forward = false;
	}

	if (se.value == VK_DOWN && Key_IsDown(in->keys, se.value)) {
		backward = true;
	} else {
		backward = false;
	}

	if (se.value == VK_LEFT && Key_IsDown(in->keys, se.value)) {
		turn_left = true;
	} else {
		turn_left = false;
	}

	if (se.value == VK_RIGHT && Key_IsDown(in->keys, se.value)) {
		turn_right = true;
	} else {
		turn_right = false;
	}

	if (se.value == VK_RETURN) {
		reset = true;
	}
#endif
}

void Com_RunEventLoop(Input *in) {
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

void Com_RunFrame(Platform *pf) {
	Sys_GenerateEvents();
	Com_RunEventLoop(pf->input);

	// test stuff
	if (paused) {
		return;
	}

	static b32 first_run = true;

	// FIXME: atm this is unused, assuming that the hw can handle the frame rate
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

	// FIXME: will be move elsewhere
	Renderer *ren = pf->renderer;
	RenderQueue *rq = pf->renderer->queue;
	MeshObject *mos = pf->renderer->back_end->entities;

	// movement testing
	if (forward) {
		ren->current_view.world_orientation.origin = 
			ren->current_view.world_orientation.origin + (ren->current_view.world_orientation.dir * 10.0f);
		forward = false;
	}

	if (backward) {
		ren->current_view.world_orientation.origin = 
			ren->current_view.world_orientation.origin - (ren->current_view.world_orientation.dir * 10.0f);
		backward = false;
	}

	if (turn_left) {
		yaw -= 5.0f;
		//yaw = DEG2RAD(yaw);

		rot_mat_y[0][0] = cos(DEG2RAD(5.0f));
		rot_mat_y[0][1] = 0.0f;
		rot_mat_y[0][2] = sin(DEG2RAD(5.0f));

		rot_mat_y[1][0] = 0.0f;
		rot_mat_y[1][1] = 1.0f;
		rot_mat_y[1][2] = 0.0f;

		rot_mat_y[2][0] = -sin(DEG2RAD(5.0f));
		rot_mat_y[2][1] = 0.0f;
		rot_mat_y[2][2] = cos(DEG2RAD(5.0f));

		ren->current_view.world_orientation.dir[0] = sinf(DEG2RAD(yaw));
		ren->current_view.world_orientation.dir[2] = cosf(DEG2RAD(yaw));

		Vec3 *verts = mos[0].mesh->local_verts->vert_array;
		for (int i = 0; i < mos[0].status.num_verts; ++i) {
			Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
		}

		turn_left = false;
	}

	if (turn_right) {
		yaw += 5.0f;
		//yaw = DEG2RAD(yaw);

		rot_mat_y[0][0] = cos(DEG2RAD(5.0f));
		rot_mat_y[0][1] = 0.0f;
		rot_mat_y[0][2] = -sin(DEG2RAD(5.0f));

		rot_mat_y[1][0] = 0.0f;
		rot_mat_y[1][1] = 1.0f;
		rot_mat_y[1][2] = 0.0f;

		rot_mat_y[2][0] = sin(DEG2RAD(5.0f));
		rot_mat_y[2][1] = 0.0f;
		rot_mat_y[2][2] = cos(DEG2RAD(5.0f));

		ren->current_view.world_orientation.dir[0] = sinf(DEG2RAD(yaw));
		ren->current_view.world_orientation.dir[2] = cosf(DEG2RAD(yaw));

		Vec3 *verts = mos[0].mesh->local_verts->vert_array;
		for (int i = 0; i < mos[0].status.num_verts; ++i) {
			Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
		}

		turn_right = false;
	}

	if (reset) {
		Vector3Init(ren->current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
		Vector3Init(ren->current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);
		Vec3 *verts = mos[0].mesh->local_verts->vert_array;
		int num_verts = mos[0].status.num_verts;

		rot_mat_y[0][0] = cos(DEG2RAD(yaw));
		rot_mat_y[2][2] = cos(DEG2RAD(yaw));

		rot_mat_y[0][2] = sin(DEG2RAD(yaw));
		rot_mat_y[2][0] = -sin(DEG2RAD(yaw));
		if (yaw != 0.0f) {
			for (int i = 0; i < num_verts; ++i) {
				Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
			}
		}
		
		reset = false;
		yaw = 0.0f;
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
	int num_entities = pf->renderer->back_end->num_entities;
	for (int i = 0; i < num_entities; ++i) {
		MeshObject *current_mo = &mos[i];
		int num_polys = current_mo->status.num_polys;

		// clear mesh states
		// FIXME: reduce the indirection overhead
		Poly *polys = current_mo->mesh->polys->poly_array;
		for (int j = 0; j < num_polys; ++j) {
			polys[j].state = polys[j].state & (~POLY_STATE_BACKFACE);
			polys[j].state = polys[j].state & (~FCS_CULL_OUT);
		}
	}

	ExecuteRenderQueue(rq, ren);
	R_RenderView(ren);

	// hacky player third person cam test stuff
	// FIXME: 0 index hardcoded for player in the mos array
	mos[0].status.world_pos = 
		ren->current_view.world_orientation.origin + (ren->current_view.world_orientation.dir * 60.0f);
	mos[0].status.world_pos[1] -= 20.0f;

	// FIXME: will be move elsewhere
	for (int i = 0; i < num_entities; ++i) {
		MeshObject *current_mo = &mos[i];
		// FIXME: reduce the indirection overhead
		Vec3 *verts = current_mo->mesh->local_verts->vert_array;

		if (1) {
			R_RotatePoints(rot_mat_z, verts, current_mo->status.num_verts); 
			R_RotatePoints(rot_mat_x, verts, current_mo->status.num_verts); 
		}

		R_TransformModelToWorld(ren, current_mo); 

		if (i != 0) {
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

	R_EndFrame(ren);


	// time debugging, first frame will be zero
	// FIXME: will be move elsewhere
	static int last_time = Sys_GetMilliseconds();
	int	now_time = Sys_GetMilliseconds();
	int	frame_msec = now_time - last_time;
	last_time = now_time;
	{
		char buffer[64];
		sprintf_s(buffer, "View (xyz): %f %f %f\n", ren->current_view.world_orientation.origin[0],
													ren->current_view.world_orientation.origin[1],
													ren->current_view.world_orientation.origin[2]);
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
// common event queue
//
#define	MAX_COM_QUED_EVENTS 1024

static SysEvent global_com_event_queue[MAX_COM_QUED_EVENTS];
static int global_com_event_head, global_com_even_tail;

SysEvent Com_GetEvent() {
	if (global_com_event_head > global_com_even_tail) {
		global_com_even_tail++;
		return global_com_event_queue[(global_com_even_tail - 1) & (MAX_COM_QUED_EVENTS - 1)];
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


