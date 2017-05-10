#include "common.h"
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

static void InitStackMemory(MemoryStack *ms, size_t num_bytes) {
	void *ptr = VirtualAlloc(0, num_bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	ms->base_ptr = (byte *)ptr;
	ms->max_size = num_bytes;
	ms->bytes_used = 0;

	if (!ms->base_ptr) {
		Sys_Print("Failed to init the stack allocator\n");
		Com_Quit();
	}
}

/*
==============================================================

COMMON

==============================================================
*/

static r32 global_game_time_residual;
static int global_game_frame;

static r32 yaw = 0.0f;
Platform Com_Init() {
	Platform pf = {};

	Sys_Init();
	pf.file_ptrs.free_file = DebugFreeFile;
	pf.file_ptrs.read_file = DebugReadFile;
	pf.file_ptrs.write_file = DebugWriteFile;

	// FIXME: make a single double ended stack, with temp allocations coming from the other side
	InitStackMemory(&pf.main_memory_stack.perm_data, MAX_PERM_MEMORY);
	InitStackMemory(&pf.main_memory_stack.temp_data, MAX_TEMP_MEMORY);

	pf.game_state = PushStruct(&pf.main_memory_stack.perm_data, GameState);
	pf.game_state->num_entities = (32);

	pf.input_state = PushStruct(&pf.main_memory_stack.perm_data, Input);

	return pf;
}

void Com_LoadEntities(Platform *pf) {
	// FIXME: testing entity stuff!!
	// FIXME: asset streaming
	Entity common_ent = {};
	common_ent.type_enum = EntityType_player;
	FileInfo cube_assets = pf->file_ptrs.read_file("cube1.plg");
	int num_entities = pf->game_state->num_entities;

	// FIXME: testing!!
	// FIXME: 0 hardcoded for player for now
	Assert(PLG_LoadMesh(&common_ent, cube_assets.data, cube_assets.size));
	memcpy(&pf->game_state->entities[0], &common_ent, sizeof(Entity));
	pf->game_state->entities[0].type_enum = EntityType_player;

	for (int i = 1; i < (num_entities); ++i) {
		Vec3 world_pos = {-100.0f + (i * 50.0f), -20.0f, 500.0f};
		memcpy(&pf->game_state->entities[i], &common_ent, sizeof(Entity));
		pf->game_state->entities[i].status.world_pos = world_pos;
		pf->game_state->entities[i].type_enum = EntityType_cube;
	}

	pf->file_ptrs.free_file(&cube_assets);
}

static void ResetEntities(RendererBackend *rb) {
	int num_polys = rb->num_polys;
	// clear mesh states
	for (int j = 0; j < num_polys; ++j) {
		rb->polys[j].state &= (~POLY_STATE_BACKFACE);
		rb->polys[j].state &= (~CULL_OUT);
	}
	// reset poly arrays
	rb->num_polys = 0;
	rb->num_verts = 0;
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

static void Com_SimFrame(r32 dt, r32 dt_residual, int num_frames, int num_entities, Entity *ents, Input *in, ViewSystem *current_view) {
	// test stuff
	static r32 rot_mat_y[3][3];
	// FIXME: add matrix returning routines
	rot_mat_y[1][0] = 0.0f;
	rot_mat_y[1][1] = 1.0f;
	rot_mat_y[1][2] = 0.0f;

	// FIXME: scaling of the world
	r32 speed = 500.0f;

	for (int i = 0; i < num_frames; ++i) {
		for (int j = 0; j < num_entities; ++j) {
			if (auto player = GetAnonType(&ents[j], player, EntityType_)) {
				Vec3 *verts = player->local_vertex_array;
				Vec3 acc = {};
				if (in->keys['W'].down) {
					acc = Vec3Norm(current_view->world_orientation.dir);
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
					acc = Vec3Norm(current_view->world_orientation.dir);
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

					// reset player yaw angle and position
					if (yaw != 0.0f) {
						for (int i = 0; i < ents[0].status.num_verts; ++i) {
							Mat1x3Mul(&verts[i], &verts[i], rot_mat_y);
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
}

#if 1
void Com_RunFrame(Platform *pf, RenderingSystem *rs) {
	Entity *entities = pf->game_state->entities;

	Sys_GenerateEvents();
	IN_GetInput(pf->input_state);
	// FIXME: move elsewhere
	if (pf->input_state->keys[ESC_KEY].released) {
		Com_Quit();
	} else if (pf->input_state->keys[SPACE_KEY].released) {
		rs->front_end.is_wireframe = !rs->front_end.is_wireframe;
	} else if (pf->input_state->keys['L'].released) {
		rs->front_end.is_ambient = (AmbientState)(!rs->front_end.is_ambient);
	}

	// FIXME: no event processing is being done atm 
	Com_RunEventLoop();

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

	Com_SimFrame((MSEC_PER_SIM / 1000.0f),
				global_game_time_residual,
				(num_game_frames_to_run > 5) ? 5 : num_game_frames_to_run,
				pf->game_state->num_entities,
				entities, pf->input_state,
				&rs->front_end.current_view);

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


	//if (rs->front_end.is_view_changed || first_run) {
	R_RenderView(&rs->front_end.current_view);
	//}
	R_BeginFrame(rs->back_end.target, &rs->back_end.cmds);

	int num_entities = pf->game_state->num_entities;
	Vec3 *local_verts = 0, *trans_verts = 0;
	Poly *polys = 0;
	// FIXME: rethink on this style of vert transforms
	for (int i = 0; i < num_entities; ++i) {
		if (auto sub_type = GetAnonType(&entities[i], player, EntityType_)) {
			local_verts = sub_type->local_vertex_array;
			trans_verts = sub_type->trans_vertex_array;

			// hacky player third person cam test stuff
			entities[i].status.world_pos = 
				rs->front_end.current_view.world_orientation.origin + (rs->front_end.current_view.world_orientation.dir * 60.0f);
			entities[i].status.world_pos[1] -= 20.0f;

			// FIXME: combine these two
			R_TransformModelToWorld(local_verts, trans_verts, ArrayCount(sub_type->local_vertex_array), entities[i].status.world_pos); 
			R_TransformWorldToView(&rs->front_end.current_view, trans_verts, ArrayCount(sub_type->trans_vertex_array));
			R_AddPolys(&rs->back_end, trans_verts, sub_type->polys, ArrayCount(sub_type->polys));
		} else {
			int num_local_verts = 0; 
			int num_trans_verts = 0; 
			int num_polys = 0;
			if (auto sub_type = GetAnonType(&entities[i], cube, EntityType_)) {
				local_verts = sub_type->local_vertex_array;
				trans_verts = sub_type->trans_vertex_array;
				polys = sub_type->polys;

				num_local_verts = ArrayCount(sub_type->local_vertex_array);
				num_trans_verts = ArrayCount(sub_type->trans_vertex_array);
				num_polys = ArrayCount(sub_type->polys);
			} else {
				InvalidCodePath("Unhandled entitity type!");
			} 
			R_RotatePoints(rot_mat_z, local_verts, num_local_verts); 
			R_RotatePoints(rot_mat_x, local_verts, num_local_verts); 
			R_TransformModelToWorld(local_verts, trans_verts, num_local_verts, entities[i].status.world_pos); 
			entities[i].status.state = (u16)R_CullPointAndRadius(&rs->front_end.current_view, entities[i].status.world_pos);			
			if (!(entities[i].status.state & CULL_OUT)) {
				R_TransformWorldToView(&rs->front_end.current_view, trans_verts, num_trans_verts);
				R_AddPolys(&rs->back_end, trans_verts, polys, num_polys);
			}
		}
	}
	R_CullBackFaces(&rs->front_end.current_view,
					rs->back_end.polys,
					rs->back_end.poly_verts,
					rs->back_end.num_polys);
	R_CalculateLighting(&rs->back_end, rs->back_end.lights, rs->front_end.is_ambient);

	// FIXME: combine view and screen transforms
	R_TransformViewToClip(&rs->front_end.current_view, rs->back_end.poly_verts, rs->back_end.num_verts);
	R_TransformClipToScreen(&rs->front_end.current_view, rs->back_end.poly_verts, rs->back_end.num_verts);

	R_PushPolysCmd(rs->back_end.target, 
				  &rs->back_end.cmds,
				  rs->back_end.polys,
				  rs->back_end.poly_verts,
				  rs->back_end.num_polys,
				  rs->front_end.is_wireframe);

	{
		static int last_time = Sys_GetMilliseconds();
		int	now_time = Sys_GetMilliseconds();
		int	frame_msec = now_time - last_time;
		last_time = now_time;

		// FIXME: v-sync when switching to hw-accelerated rendering
		//if (frame_msec > 0.016f) {
		R_EndFrame(rs->back_end.target, &rs->back_end.cmds);

		ResetEntities(&rs->back_end);
		//}

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








