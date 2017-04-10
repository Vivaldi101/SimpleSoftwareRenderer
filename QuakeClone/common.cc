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
Platform Com_Init(void *hinstance, void *wndproc) {
	Platform pf = {};

	Sys_Init();

	// FIXME: macros for memory sizes
	// FIXME: make a single double ended stack, with temp allocations coming from the other side
	pf.main_memory_stack.perm_data = InitStackMemory(MAX_PERM_MEMORY);
	pf.main_memory_stack.temp_data = InitStackMemory(MAX_TEMP_MEMORY);
	pf.game_state = PushStruct(pf.main_memory_stack.perm_data, GameState);

	// TESTING!!!!!
	pf.game_state->num_entities = 10;

	//IN_ClearKeyStates(pf.input);

	return pf;
}

void Com_LoadEntities(GameState *gs, RendererBackend *rb) {
	// FIXME: testing entity stuff!!
	// FIXME: move file api elsewhere
	// FIXME: maybe replace the CRT file i/o with win32 api

	// FIXME: add a proper filesystem
	FILE *fp;
	fopen_s(&fp, "poly_data.plg", "r");
	Sys_Print("\nTrying to open poly_data.plg file\n");

	if (!fp) {
		Sys_Print("\nCould not open poly_data.plg file\n");
		Sys_Quit();
	}
	// FIXME: testing!!
	// FIXME: 0 hardcoded for player for now
	Entity *player = &gs->entities[0];
	player->type_enum = EntityType_player;
	PLG_LoadMesh(player, &fp);

	for (int i = 1; i < gs->num_entities / 2; ++i) {
		Vec3 world_pos = {-100.0f + (i * 50.0f), -20.0f, 500.0f};
		Entity *ent = &gs->entities[i];
		PLG_LoadMesh(ent, &fp);
		ent->status.world_pos = world_pos;
	}

	if (fp) {
		Sys_Print("\nClosing poly_dat.plg file\n");
		fclose(fp);
		fp = 0;
	}

	fopen_s(&fp, "tower1.plg", "r");
	Sys_Print("\nTrying to open tower1.plg file\n");
	if (!fp) {
		Sys_Print("\nCould not open tower1.plg file\n");
		Sys_Quit();
	}

	for (int i = gs->num_entities / 2; i < gs->num_entities; ++i) {
		Vec3 world_pos = {-100.0f + (i * 50.0f), -20.0f, 200.0f};
		Entity *ent = &gs->entities[i];
		PLG_LoadMesh(ent, &fp);
		ent->status.world_pos = world_pos;
	}

	if (fp) {
		Sys_Print("\nClosing tankg1.plg file\n");
		fclose(fp);
		fp = 0;
	}
}

static void ResetEntities(Poly *polys, int num_polys) {
	// clear mesh states
	for (int j = 0; j < num_polys; ++j) {
		polys[j].state &= (~POLY_STATE_BACKFACE);
		polys[j].state &= (~FCS_CULL_OUT);
	}
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

// FIXME: pass only camera information rather than the renderer frontend
static void Com_SimFrame(r32 dt, r32 dt_residual, int num_frames, int num_entities, Entity *ents, Input *in, ViewSystem *current_view) {
	// test stuff
	static r32 rot_mat_y[3][3];
	// FIXME: add matrix returning routines
	rot_mat_y[1][0] = 0.0f;
	rot_mat_y[1][1] = 1.0f;
	rot_mat_y[1][2] = 0.0f;

	// FIXME: scaling of the world
	r32 speed = 800.0f;
	Vec3 acc = Vec3Norm(current_view->world_orientation.dir);

	for (int i = 0; i < num_frames; ++i) {
		for (int j = 0; j < num_entities; ++j) {
			if (auto player = GetAnonType(&ents[j], player, EntityType_)) {
				Vec3 *verts = player->local_vertex_array;
				if (in->keys['W'].down) {
					acc = acc * 1.0f;
				}
				if (in->keys['A'].down) {
					yaw -= 3.0f;
					// FIXME: add matrix returning routines
					rot_mat_y[0][0] = cos(DEG2RAD(3.0f));
					rot_mat_y[0][2] = sin(DEG2RAD(3.0f));

					rot_mat_y[2][0] = -rot_mat_y[0][2];
					rot_mat_y[2][2] = rot_mat_y[0][0];

					current_view->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
					current_view->world_orientation.dir[2] = cosf(DEG2RAD(yaw));

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
					rot_mat_y[0][2] = -sin(DEG2RAD(3.0f));

					rot_mat_y[2][0] = -rot_mat_y[0][2];
					rot_mat_y[2][2] = rot_mat_y[0][0];

					current_view->world_orientation.dir[0] = sinf(DEG2RAD(yaw));
					current_view->world_orientation.dir[2] = cosf(DEG2RAD(yaw));

					for (int i = 0; i < ents[0].status.num_verts; ++i) {
						Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
					}
				}
				if (in->keys[ENTER_KEY].released) {
					Vec3Init(current_view->world_orientation.dir, 0.0f, 0.0f, 1.0f);
					Vec3Init(current_view->world_orientation.origin, 0.0f, 0.0f, 0.0f);
					
					rot_mat_y[0][0] = cos(DEG2RAD(yaw));
					rot_mat_y[0][2] = sin(DEG2RAD(yaw));

					rot_mat_y[2][0] = -rot_mat_y[0][2];
					rot_mat_y[2][2] = rot_mat_y[0][0];

					// reset player yaw angle
					if (yaw != 0.0f) {
						for (int i = 0; i < ents[0].status.num_verts; ++i) {
							Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
						}
					}

					yaw = 0.0f;
				}

				acc = acc * speed;
				if (in->keys['W'].down || in->keys['S'].down) {
					current_view->world_orientation.origin = (acc * 0.5f * Square(dt)) + (current_view->velocity * dt) + current_view->world_orientation.origin;
					current_view->velocity = (acc * dt) + current_view->velocity;
				} else {
					current_view->velocity = current_view->velocity * 0.0f;
				}
			} 
		}
	}
}

#if 1
void Com_RunFrame(Platform *pf, RenderingSystem *rs) {
	Entity *entities = pf->game_state->entities;
	RendererFrontend *rfe = &rs->front_end;
	RendererBackend *rbe = &rs->back_end;
	RenderCommands *cmds = &rs->back_end.cmds;

	Sys_GenerateEvents();
	IN_GetInput(&pf->input_state);
	if (pf->input_state.keys[ESC_KEY].released) {
		Sys_Quit();
	}
	if (pf->input_state.keys[SPACE_KEY].released) {
		rfe->is_wireframe = !rfe->is_wireframe;
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

	Com_SimFrame(MSEC_PER_SIM / 1000.0f, global_game_time_residual, num_game_frames_to_run, pf->game_state->num_entities, entities, &pf->input_state, &rfe->current_view);

	// FIXME: add matrix returning routines
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


	R_RenderView(&rfe->current_view);

	// hacky player third person cam test stuff
	entities[0].status.world_pos = 
		rfe->current_view.world_orientation.origin + (rfe->current_view.world_orientation.dir * 60.0f);
	entities[0].status.world_pos[1] -= 20.0f;

	R_BeginFrame(rbe->vid_sys, cmds);

	int num_entities = pf->game_state->num_entities;
	// FIXME: 0 index hardcoded for player in the entities array for now
	// FIXME: extract these!!!
	for (int i = 0; i < num_entities; ++i) {
		if (auto ent = GetAnonType(&entities[i], tower, EntityType_)) {
			Vec3 *local_verts = ent->local_vertex_array;
			Vec3 *trans_verts = ent->trans_vertex_array;

			R_RotatePoints(rot_mat_z, local_verts, ArrayCount(ent->local_vertex_array)); 
			R_RotatePoints(rot_mat_x, local_verts, ArrayCount(ent->local_vertex_array)); 
			R_TransformModelToWorld(local_verts, trans_verts, ArrayCount(ent->local_vertex_array), entities[i].status.world_pos); 
			entities[i].status.state = R_CullPointAndRadius(&rfe->current_view, entities[i].status.world_pos);			
			if (!(entities[i].status.state & FCS_CULL_OUT)) {
				R_TransformWorldToView(&rfe->current_view, trans_verts, ArrayCount(ent->trans_vertex_array));
				// NOTE: 3 for triangles(only primitive currently drawable)
				R_AddPolys(rbe, trans_verts, ent->polys, 3, ArrayCount(ent->polys));
			}
		} else if (auto ent = GetAnonType(&entities[i], cube, EntityType_)) {
			Vec3 *local_verts = ent->local_vertex_array;
			Vec3 *trans_verts = ent->trans_vertex_array;

			R_RotatePoints(rot_mat_z, local_verts, ArrayCount(ent->local_vertex_array)); 
			R_RotatePoints(rot_mat_x, local_verts, ArrayCount(ent->local_vertex_array)); 
			R_TransformModelToWorld(local_verts, trans_verts, ArrayCount(ent->local_vertex_array), entities[i].status.world_pos); 
			entities[i].status.state = R_CullPointAndRadius(&rfe->current_view, entities[i].status.world_pos);			
			if (!(entities[i].status.state & FCS_CULL_OUT)) {
				R_TransformWorldToView(&rfe->current_view, trans_verts, ArrayCount(ent->trans_vertex_array));
				// NOTE: 3 for triangles(only primitive currently drawable)
				R_AddPolys(rbe, trans_verts, ent->polys, 3, ArrayCount(ent->polys));
			}
		} else if (auto ent = GetAnonType(&entities[i], player, EntityType_)) {
			Vec3 *local_verts = ent->local_vertex_array;
			Vec3 *trans_verts = ent->trans_vertex_array;

			R_TransformModelToWorld(local_verts, trans_verts, ArrayCount(ent->local_vertex_array), entities[i].status.world_pos); 
			R_TransformWorldToView(&rfe->current_view, trans_verts, ArrayCount(ent->trans_vertex_array));
			// NOTE: 3 for triangles(only primitive currently drawable)
			R_AddPolys(rbe, trans_verts, ent->polys, 3, ArrayCount(ent->polys));
		}
	}
	R_CullBackFaces(&rfe->current_view, rbe->polys, rbe->poly_verts, rbe->num_polys);
	R_TransformViewToClip(&rfe->current_view, rbe->poly_verts, rbe->num_verts);
	R_TransformClipToScreen(&rfe->current_view, rbe->poly_verts, rbe->num_verts);

	R_AddDrawPolysCmd(rbe->vid_sys, cmds, rbe->polys, rbe->poly_verts, rbe->num_polys, rfe->is_wireframe);

	R_EndFrame(rbe->vid_sys, cmds);

	// FIXME: move these into ending routine
	ResetEntities(rbe->polys, rbe->num_polys);
	rbe->num_polys = 0;
	rbe->num_verts = 0;

	{
		static int last_time = Sys_GetMilliseconds();
		int	now_time = Sys_GetMilliseconds();
		int	frame_msec = now_time - last_time;
		last_time = now_time;
		char buffer[64];
		sprintf_s(buffer, "Frame msec %d\n", frame_msec);
		OutputDebugStringA(buffer);
	}

	if (first_run) {
		first_run = false;
	}
}
#endif

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







