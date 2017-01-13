#include "common.h"
#include "plg_loader.h"
#include "win_renderer.h"	// FIXME: Not include here

static int s_global_game_time_residual;
static int s_global_game_frame;

// just for prototyping purposes
static MeshObject *md;
static MeshObject *player_md;

// just for prototyping purposes
static r32 turn = 0.0f;
static r32 up_down = 0.0f;
static r32 walk = 5.0f;
static Vec3 mat_rot_y[3];

static ListAllocator *global_main_list_allocator;

void Com_Init(int argc, char **argv, void *hinstance, void *wndproc) {
	Sys_Init();
	Com_InitMemory(&global_main_list_allocator, 128 * LIST_ROW_SIZE);
	md = (MeshObject *)Allocate(global_main_list_allocator, sizeof(MeshObject));
	md->mesh = (MeshData *)Allocate(global_main_list_allocator, sizeof(MeshData));
	md->mesh->local_verts = (VertexGroup *)Allocate(global_main_list_allocator, sizeof(VertexGroup));
	md->mesh->trans_verts = (VertexGroup *)Allocate(global_main_list_allocator, sizeof(VertexGroup));
	md->mesh->polys = (PolyGroup *)Allocate(global_main_list_allocator, sizeof(PolyGroup));

	player_md = (MeshObject *)Allocate(global_main_list_allocator, sizeof(MeshObject));
	player_md->mesh = (MeshData *)Allocate(global_main_list_allocator, sizeof(MeshData));
	player_md->mesh->local_verts = (VertexGroup *)Allocate(global_main_list_allocator, sizeof(VertexGroup));
	player_md->mesh->trans_verts = (VertexGroup *)Allocate(global_main_list_allocator, sizeof(VertexGroup));
	player_md->mesh->polys = (PolyGroup *)Allocate(global_main_list_allocator, sizeof(PolyGroup));

	PLG_InitParsing("poly_data.plg", md, player_md);

	R_Init(hinstance, wndproc);
}

static void ProcessEvent(SysEvent se) {

	// ALL of this is just for prototyping purposes
	if (se.ev_value == VK_ESCAPE) {
		//Sys_Print(se.ev_value);
		Sys_Quit();
	}

	if (se.ev_value == VK_UP) {
		//fwd += 5.0f;
		//mov_x += (2.0f * sin(DEG2RAD(yaw)));
		//mov_z += (2.0f * cos(DEG2RAD(yaw)));
		global_renderer_state.current_view.world_orientation.origin = 
			global_renderer_state.current_view.world_orientation.origin + (global_renderer_state.current_view.world_orientation.dir * walk);
	} else if (se.ev_value == VK_DOWN) {
		global_renderer_state.current_view.world_orientation.origin = 
			global_renderer_state.current_view.world_orientation.origin + (-global_renderer_state.current_view.world_orientation.dir * walk);
	}

	if (se.ev_value == VK_RIGHT) {
		turn += 5.0f;
		global_renderer_state.current_view.world_orientation.dir[0] = (sinf(DEG2RAD(turn)));
		global_renderer_state.current_view.world_orientation.dir[2] = (cosf(DEG2RAD(turn)));
		mat_rot_y[0][0] = cos(DEG2RAD(5.0f));
		mat_rot_y[0][1] = 0.0f;
		mat_rot_y[0][2] = sin(DEG2RAD(5.0f));

		mat_rot_y[1][0] = 0.0f;
		mat_rot_y[1][1] = 1.0f;
		mat_rot_y[1][2] = 0.0f;

		mat_rot_y[2][0] = -sin(DEG2RAD(5.0f));
		mat_rot_y[2][1] = 0.0f;
		mat_rot_y[2][2] = cos(DEG2RAD(5.0f));
		R_RotatePoints(&mat_rot_y, player_md->mesh->local_verts->vert_array, player_md->status.num_verts); 
	} else if (se.ev_value == VK_LEFT) {
		turn -= 5.0f;
		global_renderer_state.current_view.world_orientation.dir[0] = (sinf(DEG2RAD(turn)));
		global_renderer_state.current_view.world_orientation.dir[2] = (cosf(DEG2RAD(turn)));
		mat_rot_y[0][0] = cos(DEG2RAD(5.0f));
		mat_rot_y[0][1] = 0.0f;
		mat_rot_y[0][2] = -sin(DEG2RAD(5.0f));

		mat_rot_y[1][0] = 0.0f;
		mat_rot_y[1][1] = 1.0f;
		mat_rot_y[1][2] = 0.0f;

		mat_rot_y[2][0] = sin(DEG2RAD(5.0f));
		mat_rot_y[2][1] = 0.0f;
		mat_rot_y[2][2] = cos(DEG2RAD(5.0f));
		R_RotatePoints(&mat_rot_y, player_md->mesh->local_verts->vert_array, player_md->status.num_verts); 
	}
	if (se.ev_value == VK_SPACE) {
		up_down += 5.0f;
		global_renderer_state.current_view.world_orientation.dir[1] = (sinf(DEG2RAD(up_down)));
		global_renderer_state.current_view.world_orientation.dir[2] = (cosf(DEG2RAD(up_down)));
	} else if (se.ev_value == VK_CONTROL) {
		up_down -= 5.0f;
		global_renderer_state.current_view.world_orientation.dir[1] = (sinf(DEG2RAD(up_down)));
		global_renderer_state.current_view.world_orientation.dir[2] = (cosf(DEG2RAD(up_down)));
	}
}
void Com_RunEventLoop() {
	SysEvent se;

	for (;;) {

		//if ( commandExecution ) {
		//	// execute any bound commands before processing another event
		//	cmdSystem->ExecuteCommandBuffer();
		//}

		se = Com_GetEvent();

		// if no more events are available
		if (se.ev_type == SE_NONE) {
			return;
		}
		ProcessEvent(se);
	}
}
void Com_Frame() {
	Sys_GenerateEvents();
	Com_RunEventLoop();

	int num_game_frames_to_run = 0;

	static Vec3 mat_rot_x[3];
	static Vec3 mat_rot_z[3];
	static r32 rot_theta = DEG2RAD(-1.0f);
	static r32 view_angle = 0.0f;

	for (;;) {
		const int current_frame_time = Sys_GetMilliseconds();
		static int last_frame_time = current_frame_time;	
		int delta_milli_seconds = current_frame_time - last_frame_time;
		last_frame_time = current_frame_time;

		delta_milli_seconds = Com_ModifyFrameMsec(delta_milli_seconds);

		s_global_game_time_residual += delta_milli_seconds;

		for (;;) {
			// how much time to wait before running the next frame
			if (s_global_game_time_residual < MSEC_PER_SIM) {		// should be 16 or 33 
				break;
			}
			s_global_game_time_residual -= MSEC_PER_SIM;
			s_global_game_frame++;
			num_game_frames_to_run++;
			// if there is enough residual left, we may run additional frames
		}

		if (num_game_frames_to_run > 0) {
			// ready to actually run the frames
			break;
		}
		Sys_Sleep(0);
	}

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


	int num_polys = md->status.num_polys;
	Poly *polys = md->mesh->polys->poly_array;
	for (int i = 0; i < num_polys; ++i) {
		polys[i].state = polys[i].state & (~POLY_STATE_BACKFACE);
		polys[i].state = polys[i].state & (~CULL_OUT);

		polys[i].state = polys[i].state & (~POLY_STATE_BACKFACE);
		polys[i].state = polys[i].state & (~CULL_OUT);
	}

	num_polys = player_md->status.num_polys;
	polys = player_md->mesh->polys->poly_array;
	for (int i = 0; i < num_polys; ++i) {
		polys[i].state = polys[i].state & (~POLY_STATE_BACKFACE);
		polys[i].state = polys[i].state & (~CULL_OUT);

		polys[i].state = polys[i].state & (~POLY_STATE_BACKFACE);
		polys[i].state = polys[i].state & (~CULL_OUT);
	}

	player_md->status.world_pos = 
		global_renderer_state.current_view.world_orientation.origin + (global_renderer_state.current_view.world_orientation.dir * 40.0f);
	player_md->status.world_pos[1] -= 20.0f;

	R_RotatePoints(&mat_rot_z, md->mesh->local_verts->vert_array, md->status.num_verts); 
	R_RotatePoints(&mat_rot_x, md->mesh->local_verts->vert_array, md->status.num_verts); 


	R_BeginFrame(md);
	R_TransformModelToWorld(md); 
	R_TransformModelToWorld(player_md); 
	R_RenderView();
	md->status.state = R_CullPointAndRadius(md->status.world_pos);			
	R_CullBackFaces(md);
	R_CullBackFaces(player_md);
	R_TransformWorldToView(md);
	R_TransformWorldToView(player_md);
	R_TransformViewToClip(md);
	R_TransformViewToClip(player_md);
	R_TransformClipToScreen(md);
	R_TransformClipToScreen(player_md);
	R_DrawMesh(md);
	R_DrawMesh(player_md);

	R_EndFrame();

	// time debugging, first frame will be zero
	static int last_time = Sys_GetMilliseconds();
	int	now_time = Sys_GetMilliseconds();
	int	frame_msec = now_time - last_time;
	last_time = now_time;
	{
		char buffer[64];
		sprintf_s(buffer, "Turn: %f\n", turn);
		OutputDebugStringA(buffer);
	}
}
void Com_Quit() {
	Sys_Quit();
}

// Common event queue
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

inline int Com_ModifyFrameMsec(int frame_msec) {
	int clamped_msec = MSEC_PER_SIM + MSEC_PER_SIM;
	if (frame_msec > clamped_msec) {
		frame_msec = clamped_msec;
	}

	return frame_msec;
}

static void Com_InitMemory(ListAllocator **list, size_t num_bytes) {
	InitListAllocator(list, num_bytes); 
	if (!global_main_list_allocator->data) {
		Sys_Print("Failed to allocate the requested memory\n");
		Sys_Quit();
	}
}
#if 0
void Com_Memset (void* dest, const int val, const size_t count) {
	unsigned int fill_val;

	if (count < 8) {
		__asm
		{
			mov		edx,dest
			mov		eax, val
			mov		ah,al
			mov		ebx,eax
			and		ebx, 0xffff
			shl		eax,16
			add		eax,ebx				// eax now contains pattern
			mov		ecx,count
			cmp		ecx,4
			jl		skip4
			mov		[edx],eax			// copy first dword
			add		edx,4
			sub		ecx,4
	skip4:	cmp		ecx,2
			jl		skip2
			mov		word ptr [edx],ax	// copy 2 bytes
			add		edx,2
			sub		ecx,2
	skip2:	cmp		ecx,0
			je		skip1
			mov		byte ptr [edx],al	// copy single byte
	skip1:
		}
		return;
	}

	fill_val = val;
	
	fill_val = fill_val|(fill_val<<8);
	fill_val = fill_val|(fill_val<<16);		// fill dword with 8-bit pattern

	_copyDWord ((unsigned int*)(dest),fill_val, count/4);
	
	__asm									// padding of 0-3 bytes
	{
		mov		ecx,count
		mov		eax,ecx
		and		ecx,3
		jz		skipA
		and		eax,~3
		mov		ebx,dest
		add		ebx,eax
		mov		eax,fill_val
		cmp		ecx,2
		jl		skipB
		mov		word ptr [ebx],ax
		cmp		ecx,2
		je		skipA					
		mov		byte ptr [ebx+2],al		
		jmp		skipA
skipB:		
		cmp		ecx,0
		je		skipA
		mov		byte ptr [ebx],al
skipA:
	}
}
#endif

#if 0
/*
	ZONE memory allocation
	May get remade
*/
#define	ZONEID	0x1d4a11
#define MINFRAGMENT	64

struct ZoneDebug {
	char	*label;
	char	*file;
	int		 line;
	int		 alloc_size;
};

struct MemBlock {
	MemBlock	*next, *prev;
	int			size;           // including the header and possibly tiny fragments
	int			tag;            // a tag of 0 is a free block
	int			id;        		// should be ZONEID
#ifdef ZONE_DEBUG
	ZoneDebug	zd;
#endif
};

typedef struct MemZone MemZone;
struct MemZone {
	MemBlock	block_list;	    // start / end cap for linked list
	MemBlock	*rover;
	int			size;			// total bytes malloced, including header
	int			used;			// total bytes used
};

// main zone for all "dynamic" memory allocation
MemZone	*global_main_zone;
// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
MemZone	*global_small_zone;

void Z_ResetZone(MemZone *zone, int size) {
	MemBlock *block;
	
	// set the entire zone to one free block

	zone->block_list.next = zone->block_list.prev = block =
		(MemBlock *)( (byte *)zone + sizeof(MemZone) );
	zone->block_list.tag = 1;	// in use block
	zone->block_list.id = 0;
	zone->block_list.size = 0;
	zone->rover = block;
	zone->size = size;
	zone->used = 0;
	
	block->prev = block->next = &zone->block_list;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(MemZone);
}
#endif
