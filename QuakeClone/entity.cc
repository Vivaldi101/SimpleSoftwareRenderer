#include "common.h"
#include "renderer.h"
#include "entity.h"

RenderEntity(Cube) {
	Cube_ *entity = (Cube_ *)_raw_entity_data_;
	switch (_extra_flags_) {
		case PLAYER: {
			entity->world_pos = _renderer_->front_end.current_view.world_orientation.origin + (_renderer_->front_end.current_view.world_orientation.dir * 20.0f);
			entity->world_pos[1] -= 10.0f;

         RF_TransformModelToWorld(entity->model_verts, entity->trans_verts, ArrayCount(entity->model_verts), entity->world_pos, entity->scale); 
			RF_TransformWorldToView(&_renderer_->front_end.current_view, entity->trans_verts, ArrayCount(entity->trans_verts));
			RF_AddCubePolys(&_renderer_->back_end, entity->trans_verts, global_cube_index_array, ArrayCount(global_cube_index_array), 0.25f);
		} break;
		case NPC: {
			RF_TransformModelToWorld(entity->model_verts, entity->trans_verts, ArrayCount(entity->model_verts), entity->world_pos, entity->scale); 

			int clip_code = RF_CullPointAndRadius(&_renderer_->front_end.current_view, entity->world_pos);			
			if (clip_code == CULL_IN) {
				RF_TransformWorldToView(&_renderer_->front_end.current_view, entity->trans_verts, ArrayCount(entity->trans_verts));
				RF_AddCubePolys(&_renderer_->back_end, entity->trans_verts, global_cube_index_array, ArrayCount(global_cube_index_array), 1.0f);
         } 
		} break;
		default: Assert(0);
	}
}

UpdateEntity(Cube) {
	Cube_ *entity = (Cube_ *)_raw_entity_data_;

	switch(_extra_flags_) {
		case PLAYER: {
			Vec3 acc = {};
			r32 speed = 30.0f;

			RotateAroundX(0.5f, entity->model_verts, ArrayCount(entity->model_verts)); 
         RotateAroundY(0.5f, entity->model_verts, ArrayCount(entity->model_verts)); 
			if (_in_->keys['W'].down) {
				acc = Vec3Norm(_renderer_->front_end.current_view.world_orientation.dir);
				acc = acc * 1.0f;
			}
			if (_in_->keys['S'].down) {
				acc = Vec3Norm(_renderer_->front_end.current_view.world_orientation.dir);
				acc = acc * (-1.0f);
			}
         if (_in_->keys['A'].down) {
            RotateAroundY(-1.0f, &_renderer_->front_end.current_view.world_orientation.dir, 3);
			}
			if (_in_->keys['D'].down) {
            RotateAroundY(1.0f, &_renderer_->front_end.current_view.world_orientation.dir, 3);
			}
			if (_in_->keys[ENTER_KEY].down) {
            _renderer_->front_end.current_view.world_orientation.origin[0] = 0.0f;
            _renderer_->front_end.current_view.world_orientation.origin[1] = 0.0f;
            _renderer_->front_end.current_view.world_orientation.origin[2] = 0.0f;

            _renderer_->front_end.current_view.world_orientation.dir[0] = 0.0f;
            _renderer_->front_end.current_view.world_orientation.dir[1] = 0.0f;
            _renderer_->front_end.current_view.world_orientation.dir[2] = 1.0f;
			}
			acc = Vec3Norm(acc);
			acc = acc * speed;
			acc = acc + (_renderer_->front_end.current_view.velocity * -0.95f);	// hack!!!
			_renderer_->front_end.current_view.world_orientation.origin = (acc * 0.5f * Square(_dt_)) + (_renderer_->front_end.current_view.velocity * _dt_) + _renderer_->front_end.current_view.world_orientation.origin;
			_renderer_->front_end.current_view.velocity = acc * _dt_ + _renderer_->front_end.current_view.velocity;
		} break; 
		case NPC: {
			//RotateAroundX(0.5f, entity->model_verts, ArrayCount(entity->model_verts)); 
         //RotateAroundZ(0.5f, entity->model_verts, ArrayCount(entity->model_verts)); 
			if (!_in_->keys[SHIFT_KEY].down && _in_->keys['X'].down) {
            //Rotate(rot_mat_x, entity->model_verts, ArrayCount(entity->model_verts)); 
			}
			if (!_in_->keys[SHIFT_KEY].down && _in_->keys['Y'].down) {
            //Rotate(rot_mat_y, entity->model_verts, ArrayCount(entity->model_verts)); 
			}
			if (!_in_->keys[SHIFT_KEY].down && _in_->keys['Z'].down) {
            //Rotate(rot_mat_z, entity->model_verts, ArrayCount(entity->model_verts)); 
			}
			if (_in_->keys[SHIFT_KEY].down && _in_->keys['X'].down) {
            //Mat3x3Transpose(rot_mat_x);
            //Rotate(rot_mat_x, entity->model_verts, ArrayCount(entity->model_verts)); 
         }
			if (_in_->keys[SHIFT_KEY].down && _in_->keys['Y'].down) {
            //Mat3x3Transpose(rot_mat_y);
            //Rotate(rot_mat_y, entity->model_verts, ArrayCount(entity->model_verts)); 
         }
			if (_in_->keys[SHIFT_KEY].down && _in_->keys['Z'].down) {
            //Mat3x3Transpose(rot_mat_z);
            //Rotate(rot_mat_z, entity->model_verts, ArrayCount(entity->model_verts)); 
         }
		} break; 
		default: Assert(0);
	}
}

static inline size_t GetEntitySize(EntityEnum ee) {
   size_t s = 0;
#define X(name) case(name): { s = sizeof(name##_); break; }
	switch(ee) { XMACRO }
#undef X

	return s;
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

	//RF_CalculateVertexNormals(ren->back_end.polys, ren->back_end.num_polys, ren->back_end.poly_verts, ren->back_end.num_poly_verts);
	R_CalculateLighting(&ren->back_end, ren->back_end.lights, ren->front_end.is_ambient, MV3(0.0f, 0.0f, 1.0f), MV3(0.0f, 0.0f, 0.0f));
	RF_TransformViewToClip(&ren->front_end.current_view, ren->back_end.poly_verts, ren->back_end.num_poly_verts);
	RF_TransformClipToScreen(&ren->front_end.current_view, ren->back_end.poly_verts, ren->back_end.num_poly_verts);
	R_PushPolysCmd(&ren->back_end.cmds,
				  ren->back_end.polys,
				  ren->back_end.poly_verts,
              ren->back_end.test_texture,
				  ren->back_end.num_polys);
}

static void AddEntities(BaseEntity *root_be, size_t *used_entity_memory, int *num_added_base_entities, int num_entities, Vec3 world_pos, r32 scale, EntityEnum ee, u8 extra_flags) {
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
			for (int j = 0; j < ArrayCount(global_cube_norm_vertex_array); j++) {
            p->model_verts[j].xyz = global_cube_norm_vertex_array[j].xyz;	
			}

         if (extra_flags == NPC) {
				p->world_pos = world_pos;
			}
         p->scale = scale;
			p++;
		}
	} else {
		Assert(0);
	}

	*used_entity_memory += (num_entities * GetEntitySize(ee));
	(*num_added_base_entities)++;
}


void InitEntities(Platform *pf, size_t max_entity_memory_limit) {
	Assert(pf->game_state->num_base_entities == 0);
	int num_added_base_entities = 0;
	size_t used_entity_memory = 0;

	pf->game_state->entities = (BaseEntity *)GetMemStackPos(&pf->main_memory_stack.perm_data);
	memset(pf->game_state->entities, 0, sizeof(*pf->game_state->entities));
   AddEntities(pf->game_state->entities, &used_entity_memory, &num_added_base_entities, 1, MV3(0.0f, 0.0f, 0.0f), 3.0f, Cube, PLAYER);
	AddEntities(pf->game_state->entities, &used_entity_memory, &num_added_base_entities, 1, MV3(0.0f, 0.0f, 10.0f), 1.0f, Cube, NPC);
	AddEntities(pf->game_state->entities, &used_entity_memory, &num_added_base_entities, 1, MV3(10.0f, 0.0f, 10.0f), 1.0f, Cube, NPC);


	PushArray(&pf->main_memory_stack.perm_data, used_entity_memory, byte);	
	pf->game_state->num_base_entities = num_added_base_entities;
}
