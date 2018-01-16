#ifndef ENTITY_H
#define ENTITY_H
#include "r_types.h"
static const Vec3 global_model_verts[8] = {
	{ 1, 1, 1	},
	{-1, 1, 1	},
	{-1, 1, -1	},
   	{ 1, 1, -1	},
	{ 1, -1, 1	},
	{-1, -1, 1	},
	{-1, -1, -1	},
	{ 1, -1, -1	}
};

static const Vec3 global_cube_model_index_array[12] = {
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
#define XFunc(name) static void Update##name(void *_raw_entity_data_, int _extra_flags_)

// give here the entity data types to use 
#define XMACRO X(Cube) X(Pyramid)
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

struct PositionData {
	Vec3		axis[3];		// rotation vectors
	Vec3		world_pos;
	Vec3		orientation;
	Vec3		velocity;
};

struct BaseEntity {
	EntityEnum			type;															
	int					num_entities;		// the actual number of entities of type	   
	u8					extra_flags;													
	void				*raw_entity_data;	// pointer to a specific entity: Cube_ etc	
	BaseEntity			*next;
};

struct Cube_ {
	Vec3			trans_verts[8];
	PositionData	pos_data;
};

struct Pyramid_ {
	Vec3			trans_verts[5];
	PositionData	pos_data;
};
extern void InitEntities(struct Platform *pf, size_t max_entity_memory_limit);
extern void UpdateEntities(BaseEntity *root_be);
#endif	// Header guard
