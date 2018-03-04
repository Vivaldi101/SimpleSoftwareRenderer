#include "common.h"
#include "renderer.h"
#include "entity.h"

RenderEntity(Cube) {
	Cube_ *entity = (Cube_ *)_raw_entity_data_;
	switch (_extra_flags_) {
		case PLAYER: {
			// hacky player third person cam test stuff
			entity->world_pos = _renderer_->front_end.current_view.world_orientation.origin + (_renderer_->front_end.current_view.world_orientation.dir * 50.0f);
			entity->world_pos[1] -= 10.0f;

         RF_TransformModelToWorld(entity->model_verts, entity->trans_verts, ArrayCount(entity->model_verts), entity->world_pos, entity->scale); 
			RF_TransformWorldToView(&_renderer_->front_end.current_view, entity->trans_verts, ArrayCount(entity->trans_verts));
			RF_AddPolys(&_renderer_->back_end, entity->trans_verts, global_cube_model_indices, ArrayCount(global_cube_model_indices));
		} break;
		case NPC: {
			RF_TransformModelToWorld(entity->model_verts, entity->trans_verts, ArrayCount(entity->model_verts), entity->world_pos, entity->scale); 

			int clip_code = RF_CullPointAndRadius(&_renderer_->front_end.current_view, entity->world_pos);			
			if (clip_code == CULL_IN) {
				RF_TransformWorldToView(&_renderer_->front_end.current_view, entity->trans_verts, ArrayCount(entity->trans_verts));
				RF_AddPolys(&_renderer_->back_end, entity->trans_verts, global_cube_model_indices, ArrayCount(global_cube_model_indices));
			}
		} break;
		default: Assert(0);
	}
}

UpdateEntity(Cube) {
	Cube_ *entity = (Cube_ *)_raw_entity_data_;
	// FIXME: add matrix returning routines
	r32 rot_mat_x[3][3];
	rot_mat_x[0][0] = 1.0f;
	rot_mat_x[0][1] = 0.0f;
	rot_mat_x[0][2] = 0.0f;

	r32 rot_mat_y[3][3];
	rot_mat_y[1][0] = 0.0f;
	rot_mat_y[1][1] = 1.0f;
	rot_mat_y[1][2] = 0.0f;

	r32 rot_mat_z[3][3];
	rot_mat_z[2][0] = 0.0f;
	rot_mat_z[2][1] = 0.0f;
	rot_mat_z[2][2] = 1.0f;

	// FIXME: just for testing!!!!!!!!
	r32 rot_theta = DEG2RAD(-1.0f*0.0095f);
	rot_mat_x[1][0] = 0.0f;
	rot_mat_x[1][1] = cos(rot_theta);
	rot_mat_x[1][2] = sin(rot_theta);

	rot_mat_x[2][0] = 0.0f;
	rot_mat_x[2][1] = -rot_mat_x[1][2];
	rot_mat_x[2][2] = rot_mat_x[1][1];

	rot_mat_y[0][0] = cos(rot_theta);
	rot_mat_y[0][1] = 0.0f;
	rot_mat_y[0][2] = -sin(rot_theta);

	rot_mat_y[2][0] = -rot_mat_y[0][2];
	rot_mat_y[2][1] = 0.0f;
	rot_mat_y[2][2] = rot_mat_y[0][0];

	rot_mat_z[0][0] = cos(rot_theta);
	rot_mat_z[0][1] = sin(rot_theta);
	rot_mat_z[0][2] = 0.0f;

	rot_mat_z[1][0] = -rot_mat_z[0][1];
	rot_mat_z[1][1] = rot_mat_z[0][0];
	rot_mat_z[1][2] = 0.0f;
	switch(_extra_flags_) {
		case PLAYER: {
			Vec3 acc = {};
			r32 speed = 300.0f;
			// FIXME: testing
			RotatePoints(rot_mat_x, entity->model_verts, ArrayCount(entity->model_verts)); 
			RotatePoints(rot_mat_y, entity->model_verts, ArrayCount(entity->model_verts)); 
			RotatePoints(rot_mat_z, entity->model_verts, ArrayCount(entity->model_verts)); 
			if (_in_->keys['W'].down) {
				acc = Vec3Norm(_renderer_->front_end.current_view.world_orientation.dir);
				acc = acc * 1.0f;
			}
			if (_in_->keys['S'].down) {
				acc = Vec3Norm(_renderer_->front_end.current_view.world_orientation.dir);
				acc = acc * (-1.0f);
			}
			acc = Vec3Norm(acc);
			acc = acc * speed;
			acc = acc + (_renderer_->front_end.current_view.velocity * -0.95f);	// hack!!!
			_renderer_->front_end.current_view.world_orientation.origin = (acc * 0.5f * Square(_dt_)) + (_renderer_->front_end.current_view.velocity * _dt_) + _renderer_->front_end.current_view.world_orientation.origin;
			_renderer_->front_end.current_view.velocity = acc * _dt_ + _renderer_->front_end.current_view.velocity;
		} break; 
		case NPC: {
			Vec3 acc = {};
			r32 speed = 300.0f;
			// FIXME: testing
			RotatePoints(rot_mat_x, entity->model_verts, ArrayCount(entity->model_verts)); 
			RotatePoints(rot_mat_y, entity->model_verts, ArrayCount(entity->model_verts)); 
			RotatePoints(rot_mat_z, entity->model_verts, ArrayCount(entity->model_verts)); 
			if (_in_->keys['W'].down) {
				acc = Vec3Norm(_renderer_->front_end.current_view.world_orientation.dir);
				acc = acc * 1.0f;
			}
			if (_in_->keys['S'].down) {
				acc = Vec3Norm(_renderer_->front_end.current_view.world_orientation.dir);
				acc = acc * (-1.0f);
			}
			acc = Vec3Norm(acc);
			acc = acc * speed;
			acc = acc + (_renderer_->front_end.current_view.velocity * -0.95f);	// hack!!!
			_renderer_->front_end.current_view.world_orientation.origin = (acc * 0.5f * Square(_dt_)) + (_renderer_->front_end.current_view.velocity * _dt_) + _renderer_->front_end.current_view.world_orientation.origin;
			_renderer_->front_end.current_view.velocity = acc * _dt_ + _renderer_->front_end.current_view.velocity;
			RotatePoints(rot_mat_x, entity->model_verts, ArrayCount(entity->model_verts)); 
			RotatePoints(rot_mat_y, entity->model_verts, ArrayCount(entity->model_verts)); 
			RotatePoints(rot_mat_z, entity->model_verts, ArrayCount(entity->model_verts)); 
		} break; 
		default: Assert(0);
	}
}

static inline size_t GetEntitySize(EntityEnum ee) {
#define X(name) case(name): { return sizeof(name##_); }
	switch(ee) { XMACRO }
#undef X

	return 0;
}

void UpdateEntities(GameState *gs, Renderer *ren, Input *in, r32 dt, int num_frames) {
	BaseEntity *p = gs->entities;
	int num_base_entities = gs->num_base_entities;
#define X(name) case(name): UpdateEntity##name(raw_entity_data, ren, in, dt, extra_flags); break;
	for (int i = 0; i < num_frames; i++) {
		for (int j = 0; j < num_base_entities; j++) {
			EntityEnum ee = p->type;
			int num_entities = p->num_entities;
			int extra_flags = p->extra_flags;
			byte *raw_entity_data = (byte *)p->raw_entity_data;
			for (int k = 0; k < num_entities; k++) {
				switch(ee) { XMACRO }
				raw_entity_data += GetEntitySize(ee);
			}

			p = (BaseEntity *)((byte *)p + (sizeof(BaseEntity) + (num_entities * GetEntitySize(ee))));
		}	
	}
#undef X
}

void RenderEntities(GameState *gs, Renderer *ren) {
	BaseEntity *p = gs->entities;
	int num_base_entities = gs->num_base_entities;
#define X(name) case(name): RenderEntity##name(raw_entity_data, ren, extra_flags); break;
	for (int i = 0; i < num_base_entities; i++) {
		EntityEnum ee = p->type;
		int num_entities = p->num_entities;
		int extra_flags = p->extra_flags;
		byte *raw_entity_data = (byte *)p->raw_entity_data;
		for (int i = 0; i < num_entities; i++) {
			switch(ee) { XMACRO }
			raw_entity_data += GetEntitySize(ee);
		}

		p = (BaseEntity *)((byte *)p + (sizeof(BaseEntity) + (num_entities * GetEntitySize(ee))));
	}	
#undef X

	RF_CullBackFaces(&ren->front_end.current_view, ren->back_end.polys, ren->back_end.num_polys);
	RF_CalculateVertexNormals(ren->back_end.polys, ren->back_end.num_polys, ren->back_end.poly_verts, ren->back_end.num_poly_verts);
	R_CalculateLighting(&ren->back_end, ren->back_end.lights, ren->front_end.is_ambient, MV3(0.0f, 0.0f, 1.0f), MV3(0.0f, 0.0f, 0.0f));
	RF_TransformViewToClip(&ren->front_end.current_view, ren->back_end.poly_verts, ren->back_end.num_poly_verts);
	RF_TransformClipToScreen(&ren->front_end.current_view, ren->back_end.poly_verts, ren->back_end.num_poly_verts);
}

static void AddEntities(BaseEntity *root_be, size_t *used_entity_memory, int *num_added_base_entities, int num_entities, EntityEnum ee, u8 extra_flags) {
	BaseEntity *new_be;
	byte *curr_pos;

	curr_pos = (byte *)root_be;
	while (((BaseEntity *)curr_pos)->type != Invalid) {
		curr_pos += (sizeof(BaseEntity) + (GetEntitySize(((BaseEntity *)curr_pos)->type) * ((BaseEntity *)curr_pos)->num_entities));
	}

	new_be = (BaseEntity *)curr_pos;
	new_be->type = ee;
	new_be->num_entities = num_entities;
	new_be->extra_flags = extra_flags;
	new_be->raw_entity_data = curr_pos + sizeof(BaseEntity);

	if (ee == Cube) {
		Cube_ *p = (Cube_ *)new_be->raw_entity_data;
		for (int i = 0; i < num_entities; i++) {
			for (int j = 0; j < ArrayCount(global_cube_normalized_model_verts); j++) {
				p->model_verts[j] = global_cube_normalized_model_verts[j];	
			}
			if (extra_flags == NPC) {
				Vec3 world_pos = {0.0f, 0.0f, 200.0f};	// random pos
				p->world_pos = world_pos;
			}
         p->scale = 5.0f;
			p++;
		}
	} else {
		Assert(0);
	}

	*used_entity_memory += (num_entities * GetEntitySize(ee));
	(*num_added_base_entities)++;
}


void InitEntities(Platform *pf, size_t max_entity_memory_limit) {
	int num_added_base_entities = 0;
	size_t used_entity_memory = 0;

	pf->game_state->entities = (BaseEntity *)GetMemStackPos(&pf->main_memory_stack.perm_data);
	memset(pf->game_state->entities, 0, sizeof(*pf->game_state->entities));
	//AddEntities(pf->game_state->entities, &used_entity_memory, &num_added_base_entities, 1, Cube, PLAYER);
	AddEntities(pf->game_state->entities, &used_entity_memory, &num_added_base_entities, 1, Cube, NPC);

	Assert(pf->game_state->num_base_entities == 0);

	PushArray(&pf->main_memory_stack.perm_data, used_entity_memory, byte);	
	pf->game_state->num_base_entities = num_added_base_entities;
}
