#include "common.h"
#include "plg_loader.h"
#include "win_renderer.h"	// FIXME: Not include here

int global_com_frame_time;
int global_com_frame_msec;

static int s_global_game_time_residual;
static int s_global_game_frame;

// just for prototyping purposes
static MeshData md;
static MeshData player_md;

// just for prototyping purposes
static r32 yaw = 0.0f;
static r32 mov_x = 0.0f;
static r32 mov_z = 0.0f;
void Com_Init(int argc, char **argv, void *hinstance, void *wndproc) {
	Sys_Init();

	PLG_InitParsing("poly_data.plg", &md, &player_md);

	if (!R_Init(hinstance, wndproc)) {
		Sys_Print("Error while initializing the renderer");
		Sys_Quit();
	}
}

static void ProcessEvent(SysEvent se) {
	// just for prototyping purposes
	if (se.ev_value == VK_ESCAPE) {
		//Sys_Print(se.ev_value);
		Sys_Quit();
	}
	if (se.ev_value == VK_UP) {
		//fwd += 5.0f;
		mov_x += (2.0f * sin(DEG2RAD(yaw)));
		mov_z += (2.0f * cos(DEG2RAD(yaw)));
	} else if (se.ev_value == VK_DOWN) {
		//fwd -= 5.0f;
		mov_x -= (2.0f * sin(DEG2RAD(yaw)));
		mov_z -= (2.0f * cos(DEG2RAD(yaw)));
	}
	if (se.ev_value == VK_RIGHT) {
		++yaw;
	} else if (se.ev_value == VK_LEFT) {
		--yaw;
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
	static Vec3 mat_rot_y[3];
	static Vec3 mat_rot_z[3];
	static r32 rot_theta = DEG2RAD(-1.0f);

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

	player_md.world_pos[0] = mov_x + 50.0f * sin(DEG2RAD(yaw));
	player_md.world_pos[1] = global_renderer_state.current_view.world_orientation.origin[1] - 25.0f;
	player_md.world_pos[2] = mov_z + 50.0f * cos(DEG2RAD(yaw));

	R_RotatePoints(&mat_rot_z, md.local_vertex_array, md.num_verts); 
	R_RotatePoints(&mat_rot_x, md.local_vertex_array, md.num_verts); 

	int num_polys = md.num_polys;
	for (int i = 0; i < num_polys; ++i) {
		md.poly_array[i].state = md.poly_array[i].state & (~POLY_STATE_BACKFACE);
		md.poly_array[i].state = md.poly_array[i].state & (~CULL_OUT);

		player_md.poly_array[i].state = md.poly_array[i].state & (~POLY_STATE_BACKFACE);
		player_md.poly_array[i].state = md.poly_array[i].state & (~CULL_OUT);
	}
	R_BeginFrame(&md);
	R_TransformModelToWorld(&md); 
	R_TransformModelToWorld(&player_md); 
	R_SetupEulerView(0.0f, yaw, 0.0f, mov_x, 0.0f, mov_z);
	R_SetupFrustum(90.0f, 50.0f, 500.0f);					// we should only compute the plane normals every frame
	md.state = R_CullPointAndRadius(md.world_pos, 1.0f);	// radius value is for testing for now
	R_CullBackFaces(&md);
	R_CullBackFaces(&player_md);
	R_TransformWorldToView(&md);
	R_TransformWorldToView(&player_md);
	R_TransformViewToClip(&md);
	R_TransformViewToClip(&player_md);
	R_TransformClipToScreen(&md);
	R_TransformClipToScreen(&player_md);
	R_DrawMesh(&md);
	R_DrawMesh(&player_md);

	//R_DrawGradient(&global_renderer_state.vid_sys);
	R_EndFrame();

	// time debugging, first frame will be zero
	static int last_time = Sys_GetMilliseconds();
	int	now_time = Sys_GetMilliseconds();
	int	frame_msec = now_time - last_time;
	last_time = now_time;
	{
		char buffer[64];
		sprintf_s(buffer, "MS: %d, yaw: %f\n", frame_msec, yaw);
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
