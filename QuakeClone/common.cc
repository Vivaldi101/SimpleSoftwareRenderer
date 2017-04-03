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

static b32 paused;
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

static void ProcessEvent(SysEvent se) {
}

static void Com_RunEventLoop(GameState *gs, Input *in) {
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

static void Com_SimFrame(r32 dt, r32 dt_residual, int num_frames, Entity *ents, Input *in, ViewSystem *vs) {
	// test stuff
	static r32 rot_mat_y[3][3];
	// FIXME: add matrix returning routines
	rot_mat_y[1][0] = 0.0f;
	rot_mat_y[1][1] = 1.0f;
	rot_mat_y[1][2] = 0.0f;
	r32 speed = 800.0f;
	Vec3 acc = Vec3Norm(vs->world_orientation.dir);

	for (int i = 0; i < num_frames; ++i) {
		if (in->keys['W'].down) {
			acc = acc * 1.0f;
		}
		if (in->keys['A'].down) {
			yaw -= 3.0f;
			// FIXME: add matrix returning routines
			rot_mat_y[0][0] = cos(DEG2RAD(3.0f));
			rot_mat_y[0][1] = 0.0f;
			rot_mat_y[0][2] = sin(DEG2RAD(3.0f));

			rot_mat_y[2][0] = -sin(DEG2RAD(3.0f));
			rot_mat_y[2][1] = 0.0f;
			rot_mat_y[2][2] = cos(DEG2RAD(3.0f));

			vs->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
			vs->world_orientation.dir[2] = cosf(DEG2RAD(yaw));

			Vec3 *verts = ents[0].mesh->local_verts->vert_array;
			for (int i = 0; i < ents[0].status.num_verts; ++i) {
				Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
			}
		}
		if (in->keys['S'].down) {
			acc = acc * (-1.0f);
		}
		if (in->keys['D'].down) {
			yaw += 3.0f;
			// FIXME: add matrix returning routines
			rot_mat_y[0][0] = cos(DEG2RAD(3.0f));
			rot_mat_y[0][1] = 0.0f;
			rot_mat_y[0][2] = -sin(DEG2RAD(3.0f));

			rot_mat_y[2][0] = sin(DEG2RAD(3.0f));
			rot_mat_y[2][1] = 0.0f;
			rot_mat_y[2][2] = cos(DEG2RAD(3.0f));

			vs->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
			vs->world_orientation.dir[2] = cosf(DEG2RAD(yaw));

			Vec3 *verts = ents[0].mesh->local_verts->vert_array;
			for (int i = 0; i < ents[0].status.num_verts; ++i) {
				Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
			}
		}
		if (in->keys[ENTER_KEY].released) {
			Vec3Init(vs->world_orientation.dir, 0.0f, 0.0f, 1.0f);
			Vec3Init(vs->world_orientation.origin, 0.0f, 0.0f, 0.0f);
			
			rot_mat_y[0][0] = cos(DEG2RAD(yaw));
			rot_mat_y[0][2] = sin(DEG2RAD(yaw));

			rot_mat_y[2][0] = -sin(DEG2RAD(yaw));
			rot_mat_y[2][2] = cos(DEG2RAD(yaw));

			// reset player camera
			if (yaw != 0.0f) {
				Vec3 *verts = ents[0].mesh->local_verts->vert_array;
				for (int i = 0; i < ents[0].status.num_verts; ++i) {
					Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
				}
			}

			yaw = 0.0f;
		}

		acc = acc * speed;
		if (in->keys['W'].down || in->keys['S'].down) {
			vs->world_orientation.origin = (acc * 0.5f * Square(dt)) + (vs->velocity * dt) + vs->world_orientation.origin;
			vs->velocity = (acc * dt) + vs->velocity;
		} else {
			vs->velocity = vs->velocity * 0.0f;
		}
	}
	{
		//char buffer[64];
		//sprintf_s(buffer, "Acc in z: %f\n", acc[2]);
		//OutputDebugStringA(buffer);
	}
}

void Com_RunFrame(Platform *pf, RenderingSystem *rs) {
	Entity *entities = pf->game_state->entities;
	ViewSystem *current_view = &rs->front_end.current_view;
	RendererBackend *rbe = &rs->back_end;
	RenderCommands *cmds = &rs->back_end.cmds;

	Sys_GenerateEvents();
	IN_GetInput(&pf->input_state);
	if (pf->input_state.keys[ESC_KEY].released) {
		Sys_Quit();
	}

	Com_RunEventLoop(pf->game_state, &pf->input_state);


	static b32 first_run = true;

	int num_game_frames_to_run = 0;

	r32 rot_mat_x[3][3];
	rot_mat_x[0][0] = 1.0f;
	rot_mat_x[0][1] = 0.0f;
	rot_mat_x[0][2] = 0.0f;

	r32 rot_mat_z[3][3];
	rot_mat_z[2][0] = 0.0f;
	rot_mat_z[2][1] = 0.0f;
	rot_mat_z[2][2] = 1.0f;

	r32 rot_theta = DEG2RAD(-1.0f);

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

	Com_SimFrame(MSEC_PER_SIM / 1000.0f, global_game_time_residual, num_game_frames_to_run, entities, &pf->input_state, current_view);

	// FIXME: add matrix returning routines
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
	R_AddDrawPolysCmd(rbe->vid_sys, cmds, rbe->polys, rbe->poly_verts, rbe->num_polys, true);

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
		sprintf_s(buffer, "Origin x, y, z: %f %f %f\n", current_view->world_orientation.origin[0], 
				  current_view->world_orientation.origin[1], 
				  current_view->world_orientation.origin[2]);
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




