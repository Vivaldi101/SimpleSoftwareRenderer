#ifndef ENTITY_H
#define ENTITY_H
#include "shared.h"

static const PolyVert global_cube_normalized_model_verts[8] = {
	{{1, 1, 1}		},
	{{-1, 1, 1}		},
	{{-1, 1, -1}	},
	{{1, 1, -1}		},
	{{1, -1, 1}		},
	{{-1, -1, 1}	},
	{{-1, -1, -1}	},
	{{1, -1, -1}	}
};

static const Vec3i global_cube_model_indices[12] = {
	{ 0, 1, 2 },
	{ 0, 2, 3 },
	{ 4, 7, 6 },
	{ 4, 6, 5 },
	{ 3, 2, 6 },
	{ 3, 6, 7 },
	{ 0, 4, 5 },
	{ 0, 5, 1 },
	{ 1, 5, 6 },
	{ 1, 6, 2 },
	{ 0, 3, 7 },
	{ 0, 7, 4 }
};


#define INVALID_XMACRO_CASE ((EntityEnum)-1)
#define X(name) name,
#define UpdateEntity(name) static inline void UpdateEntity##name(void *_raw_entity_data_, Renderer *_renderer_, Input *_in_, r32 _dt_, int _extra_flags_)
#define RenderEntity(name) static void RenderEntity##name(void *_raw_entity_data_, Renderer *_renderer_, int _extra_flags_)

// give here the entities to use 
#define XMACRO X(Cube)
#undef X

enum EntityEnum {
	Invalid,
#define X(name) name,
	XMACRO
#undef X
};

// how many different entities there are
#define X(name) + 1
static const int NUM_ENTITY_TYPES = (0 + XMACRO); 
#undef X

enum EntityFlags {
	PLAYER	= 1 << 0,
	NPC		= 1 << 1
};

enum {
	X_AXIS	= 0,
	Y_AXIS	= 1,
	Z_AXIS	= 2
};

//struct EntityPosition {
//	Vec3		axis[3];		// rotation vectors
//	Vec3		world_pos;
//	Vec3		orientation;
//	Vec3		velocity;
//};

struct BaseEntity {
	EntityEnum			type;															
	int					num_entities;		// the actual number of entities of type	   
	u8					extra_flags;													
	void				*raw_entity_data;	// pointer to a specific entity: Cube_ etc	
};

struct Cube_ {
	PolyVert	model_verts[8];
	PolyVert	trans_verts[8];
	//Vec3		axis[3];		// rotation vectors
	Vec3		world_pos;
	Vec3		orientation;
	Vec3		velocity;
	r32      scale;
};

extern void InitEntities(struct Platform *pf, size_t max_entity_memory_limit = (MAX_PERM_MEMORY >> 1));
extern void UpdateEntities(GameState *gs, Renderer *ren, Input *in, r32 dt, int num_frames);
extern void RenderEntities(GameState *gs, Renderer *ren);
#endif	// Header guard
