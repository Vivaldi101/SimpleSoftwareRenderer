#include "common.h"
#include "plg_loader.h"
#include "win_renderer.h"	// FIXME: Not include here


/*
==============================================================

MEMORY MANAGEMENT

==============================================================
*/

static u32 NextPO2(u32 v) {
	v--;	// handle the zero case
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}


// fixed size allocator

#define BYTE_INDEX_16	1		// index of first 16-byte payload
#define BYTE_INDEX_32	18		// index of first 16-byte payload
#define BYTE_INDEX_64	51		// index of first 16-byte payload
#define BYTE_INDEX_128	116		// index of first 16-byte payload
#define BYTE_INDEX_256	245		// index of first 16-byte payload
#define BYTE_INDEX_512	502		// index of first 16-byte payload
#define BYTE_INDEX_1024	1015	// index of first 16-byte payload
#define BYTE_INDEX_2048	2040	// index of first 16-byte payload

#define BLOCK_STATE(data, index)	((data)[(index) - 1])	// check if empty block

#define FREE_BLOCK 0xfb
#define ALLOCATED_BLOCK 0xab
#define LIST_ROW_SIZE	4088	// size of a row of entries in table

struct ListAllocator {
	byte *	data;
	size_t	num_bytes;
	int		num_rows;
	union {
	} type;
};

static void InitColumn(ListAllocator *list, int index) {
	int num_rows = list->num_rows;
	for (int i = 0; i < num_rows; ++i) {
		BLOCK_STATE(list->data, index) = FREE_BLOCK;
		index += LIST_ROW_SIZE;
	}
}

static void DestroyListAllocator(ListAllocator *list) {
	list->num_rows = 0;
	list->num_bytes = 0;
	VirtualFree(list->data, 0, MEM_RELEASE);
}

static int SearchColumn(ListAllocator *list, int index) {
	int num_rows = list->num_rows;

	for (int i = 0; i < num_rows; ++i) {
		if (BLOCK_STATE(list->data, index) == FREE_BLOCK) {
			BLOCK_STATE(list->data, index) = ALLOCATED_BLOCK;

			return index;
		}
		index += LIST_ROW_SIZE;
	}

	return 0;
}

void *Allocate(ListAllocator *la, size_t num_bytes) {
	int index;

	if (!la->data) {
		Sys_Print("List memory system not initialized\n");
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

void Free(ListAllocator *la, void **ptr) {
	int index = (int)((byte *)(*ptr) - la->data);
	index = index % LIST_ROW_SIZE;
	*ptr = 0;

	if (index >= 1 || index < la->num_bytes) {
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

static ListAllocator *InitListMemory(size_t num_bytes) {
	ListAllocator *la = (ListAllocator *)VirtualAlloc(0, num_bytes + sizeof(ListAllocator), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!la) {
		Sys_Print("Failed to init the fixed sized allocator\n");
		Sys_Quit();
	}
	la->num_bytes = num_bytes;
	la->num_rows = (int)num_bytes / LIST_ROW_SIZE;
	la->data = (byte *)la + sizeof(ListAllocator);

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

// stack based allocator
// may get re-done api-wise
#if 1
#define	ALLOC_MAGIC			0x89537892
#define	ALLOC_FREE_MAGIC	0x89537893

enum StackAllocPref {
	SAP_HIGH,
	SAP_LOW,
	SAP_ANY
};

struct StackHeader {
	int	magic;
	int	size;
};

struct StackUsed {
	int		mark;
	size_t	permanent;
	size_t	temp;
};

struct StackBlock {
	byte *			data;
	StackBlock *	next;
	StackUsed 		used;
	size_t			total_size;
	int				temp_highwater;
	b32				is_temp_block_high;
	//byte			printed;
	//char *			label;
	//char *			file;
	//int				line;
};

//static StackUsed global_stack_low, global_stack_high;
//static StackUsed *global_stack_permanent, *global_stack_temp;

static void ClearStack(StackBlock *sb) {
	sb->total_size = sb->used.permanent = sb->used.temp = 0;
	sb->is_temp_block_high = true;
//
//#ifndef DEDICATED
//	CL_ShutdownCGame();
//	CL_ShutdownUI();
//#endif
//	SV_ShutdownGameProgs();
//#ifndef DEDICATED
//	CIN_CloseAllVideos();
//#endif
//	hunk_low.mark = 0;
//	hunk_low.permanent = 0;
//	hunk_low.temp = 0;
//	hunk_low.tempHighwater = 0;
//
//	hunk_high.mark = 0;
//	hunk_high.permanent = 0;
//	hunk_high.temp = 0;
//	hunk_high.tempHighwater = 0;
//
//	hunk_permanent = &hunk_low;
//	hunk_temp = &hunk_high;
//
//	Com_Printf( "Hunk_Clear: reset the hunk ok\n" );
//	VM_Clear();
//#ifdef HUNK_DEBUG
//	hunkblocks = NULL;
//#endif
}


#ifdef ALLOC_DEBUG
void *AllocateDebug( int size, ha_pref preference, char *label, char *file, int line ) {
#else
void *Allocate(StackBlock *sb, size_t num_bytes) {
#endif
	void *buffer = 0;

	if (!sb->data) {
		Sys_Print("Stack memory system not initialized\n");
		Sys_Quit();
	}
//
//	// can't do preference if there is any temp allocated
//	if (preference == h_dontcare || hunk_temp->temp != hunk_temp->permanent) {
//		Hunk_SwapBanks();
//	} else {
//		if (preference == h_low && hunk_permanent != &hunk_low) {
//			Hunk_SwapBanks();
//		} else if (preference == h_high && hunk_permanent != &hunk_high) {
//			Hunk_SwapBanks();
//		}
//	}
//
//#ifdef ALLOC_DEBUG
//	size += sizeof(hunkblock_t);
//#endif
//
//	// round to cacheline
//	size = (size+31)&~31;
//
	if (sb->used.permanent + sb->used.temp + num_bytes > sb->total_size) {
#ifdef ALLOC_DEBUG
		Hunk_Log();
		Hunk_SmallLog();
#endif
		//Com_Error( ERR_DROP, "Hunk_Alloc failed on %i");
		Sys_Print("Stack allocation failure!");
	}

	if (sb->is_temp_block_high) {
		buffer = (void *)(sb->data + sb->used.permanent);
		sb->used.permanent += num_bytes;
	} else {
		sb->used.permanent += num_bytes;
		buffer = (void *)(sb->data + sb->total_size - sb->used.permanent);
	}

	return buffer;
}

void SlideOffStack(StackBlock *sb, void *ptr, size_t num_bytes) {
	memset(ptr, 0, num_bytes);
	size_t num_bytes_to_move = sb->total_size - num_bytes;
	memmove(ptr, (byte *)ptr + num_bytes, num_bytes_to_move);
	sb->used.permanent -= num_bytes;
}

#endif

static StackBlock *InitStackMemory(size_t num_bytes) {
	// make sure the file system has allocated and "not" freed any temp blocks
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redunant routines in the file system utilizing different 
	// memory systems

	//if (FS_LoadStack() != 0) {
	//	Com_Error( ERR_FATAL, "Hunk initialization failed. File system load stack not zero");
	//}

	StackBlock *sb = (StackBlock *)VirtualAlloc(0, num_bytes + sizeof(StackBlock), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!sb) {
		Sys_Print("Failed to init the stack allocator\n");
		Sys_Quit();
	}
	ClearStack(sb);
	sb->total_size = num_bytes;
	sb->data = (byte *)sb + sizeof(StackBlock);

	return sb;
}


static int global_game_time_residual;
static int global_game_frame;

// just for prototyping purposes
//static r32 up_down = 0.0f;

static r32 yaw = 0.0f;
static b32 forward;
static b32 backward;

static b32 turn_left;
static b32 turn_right;

static Vec3 mat_rot_y[3];

static b32 paused;
static b32 reset;

EngineData Com_InitEngine(void *hinstance, void *wndproc) {
	EngineData ed = {};
	MeshObject *mo;
	MeshObject *player_mo;

	Sys_Init();

	ed.list_allocator = InitListMemory(1024 * LIST_ROW_SIZE);
	ed.stack_allocator = InitStackMemory(MEGABYTES(32));
	R_Init(&ed, hinstance, wndproc);


	// just for prototyping purposes
	player_mo = (MeshObject *)Allocate(ed.list_allocator, sizeof(MeshObject));
	player_mo->mesh = (MeshData *)Allocate(ed.list_allocator, sizeof(MeshData));
	player_mo->mesh->local_verts = (VertexGroup *)Allocate(ed.list_allocator, sizeof(VertexGroup));
	player_mo->mesh->trans_verts = (VertexGroup *)Allocate(ed.list_allocator, sizeof(VertexGroup));
	player_mo->mesh->polys = (PolyGroup *)Allocate(ed.list_allocator, sizeof(PolyGroup));

	// FIXME: move file api elsewhere
	// just for prototyping purposes
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
	memcpy(&ed.renderer->back_end->entities[ed.renderer->back_end->num_entities++], player_mo, sizeof(MeshObject));

	int num_entities = 5;
	for (int i = 0; i < num_entities; ++i) {
		// just for prototyping purposes
		mo = (MeshObject *)Allocate(ed.list_allocator, sizeof(MeshObject));
		mo->mesh = (MeshData *)Allocate(ed.list_allocator, sizeof(MeshData));
		mo->mesh->local_verts = (VertexGroup *)Allocate(ed.list_allocator, sizeof(VertexGroup));
		mo->mesh->trans_verts = (VertexGroup *)Allocate(ed.list_allocator, sizeof(VertexGroup));
		mo->mesh->polys = (PolyGroup *)Allocate(ed.list_allocator, sizeof(PolyGroup));


		// FIXME: move elsewhere
		Vec3 world_pos = {-100.0f + (i * 50.0f), 20.0f, 200.0f};
		PLG_LoadMeshObject(mo, &fp, world_pos);

		memcpy(&ed.renderer->back_end->entities[ed.renderer->back_end->num_entities++], mo, sizeof(MeshObject));
	}

	if (fp) {
		Sys_Print("\nClosing PLG file\n");
		fclose(fp);
	}


	return ed;
}

static void ProcessEvent(SysEvent se) {

	// ALL of this is just for prototyping purposes
	if (se.ev_value == VK_ESCAPE) {
		//Sys_Print(se.ev_value);
		Sys_Quit();
	}

	if (se.ev_value == VK_PAUSE) {
		paused = !paused;
	}

	// FIXME: move input handling elsewhere
	if (se.ev_value == VK_UP) {
		forward = true;
	} 

	if (se.ev_value == VK_DOWN) {
		backward = true;
	} 

	if (se.ev_value == VK_RIGHT) {
		turn_right = true;
	} else if (se.ev_value == VK_LEFT) {
		turn_left = true;
	}

	if (se.ev_value == VK_RETURN) {
		reset = true;
	}
}

void Com_RunEventLoop() {
	SysEvent se;

	for (;;) {
		se = Com_GetEvent();

		// if no more events 
		if (se.ev_type == SET_NONE) {
			return;
		}
		ProcessEvent(se);
	}
}

void Com_RunFrame(EngineData *ed) {
	Sys_GenerateEvents();
	Com_RunEventLoop();

	// test stuff
	if (paused) {
		return;
	}

	static b32 first_run = true;

	// FIXME: atm this is unused
	int num_game_frames_to_run = 0;

	// test stuff
	r32 mat_rot_x[3][3];
	r32 mat_rot_z[3][3];
	static r32 rot_theta = DEG2RAD(-1.0f);
	static r32 view_angle = 0.0f;

	for (;;) {
		const int current_frame_time = Sys_GetMilliseconds();
		static int last_frame_time = current_frame_time;	
		int delta_milli_seconds = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		delta_milli_seconds = Com_ModifyFrameMsec(delta_milli_seconds);

		global_game_time_residual += delta_milli_seconds;

		for (;;) {
			// how much time to wait before running the next frame
			if (global_game_time_residual < MSEC_PER_SIM) {		// should be 16 or 33 
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
	Renderer *ren = ed->renderer;

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
		ren->current_view.world_orientation.dir[0] = (sinf(DEG2RAD(yaw)));
		ren->current_view.world_orientation.dir[2] = (cosf(DEG2RAD(yaw)));
		turn_left = false;
	}

	if (turn_right) {
		yaw += 5.0f;
		ren->current_view.world_orientation.dir[0] = (sinf(DEG2RAD(yaw)));
		ren->current_view.world_orientation.dir[2] = (cosf(DEG2RAD(yaw)));
		turn_right = false;
	}

	if (reset) {
		yaw = 0.0f;
		Vector3Init(ren->current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
		Vector3Init(ren->current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);
		reset = false;
	}


	// test stuff
	mat_rot_x[0][0] = 1.0f;
	mat_rot_x[0][1] = 0.0f;
	mat_rot_x[0][2] = 0.0f;

	mat_rot_x[1][0] = 0.0f;
	mat_rot_x[1][1] = cos(rot_theta);
	mat_rot_x[1][2] = sin(rot_theta);

	mat_rot_x[2][0] = 0.0f;
	mat_rot_x[2][1] = -sin(rot_theta);
	mat_rot_x[2][2] = cos(rot_theta);


	mat_rot_z[0][0] = cos(rot_theta);
	mat_rot_z[0][1] = sin(rot_theta);
	mat_rot_z[0][2] = 0.0f;

	mat_rot_z[1][0] = -sin(rot_theta);
	mat_rot_z[1][1] = cos(rot_theta);
	mat_rot_z[1][2] = 0.0f;

	mat_rot_z[2][0] = 0.0f;
	mat_rot_z[2][1] = 0.0f;
	mat_rot_z[2][2] = 1.0f;


	// test stuff
	MeshObject *mos = ed->renderer->back_end->entities;
	int num_entities = ed->renderer->back_end->num_entities;
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

	R_BeginFrame(ren);
	R_RenderView(ren);

	// hacky player third person cam test stuff
	// FIXME: 0 hardcoded index for player
	mos[0].status.world_pos = 
		ren->current_view.world_orientation.origin + (ren->current_view.world_orientation.dir * 45.0f);
	mos[0].status.world_pos[1] -= 20.0f;


	// FIXME: will be move elsewhere
	for (int i = 0; i < num_entities; ++i) {
		MeshObject *current_mo = &mos[i];
		// FIXME: reduce the indirection overhead
		Vec3 *verts = current_mo->mesh->local_verts->vert_array;

		R_RotatePoints(mat_rot_z, verts, current_mo->status.num_verts); 
		R_RotatePoints(mat_rot_x, verts, current_mo->status.num_verts); 

		R_TransformModelToWorld(ren, current_mo); 

		//current_mo->status.state = R_CullPointAndRadius(current_mo->status.world_pos);			
		if (!(current_mo->status.state & FCS_CULL_OUT)) {
			R_CullBackFaces(ren, current_mo);
			R_TransformWorldToView(ren, current_mo);
			R_TransformViewToClip(ren, current_mo);
			R_TransformClipToScreen(ren, current_mo);
			R_DrawWireframeMesh(ren, current_mo);
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

// common event queue
#define	MAX_COM_QUED_EVENTS 1024

static SysEvent global_com_event_queue[MAX_COM_QUED_EVENTS];
static int global_com_event_head, global_com_eventail;

SysEvent Com_GetEvent() {
	if (global_com_event_head > global_com_eventail) {
		global_com_eventail++;
		return global_com_event_queue[(global_com_eventail - 1) & (MAX_COM_QUED_EVENTS - 1)];
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


