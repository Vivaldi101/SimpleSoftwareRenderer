#ifndef ENTITY_H
#define ENTITY_H
#include "shared.h"

static const PolyVert global_cube_norm_vertex_array[8] = {
	{1, 1, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{-1, 1, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{-1, 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{1, 1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{1, -1, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{-1, -1, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{-1, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
	{1, -1, -1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

static u16 global_cube_index_array[12][3] = {
	{0, 1, 2},   // top
	{0, 2, 3},   // top
	{4, 7, 6},   // bottom
	{4, 6, 5},   // bottom
	{3, 2, 6},   // front
	{3, 6, 7},   // front
	{0, 4, 5},   // back
	{0, 5, 1},   // back
	{1, 5, 6},   // left
	{1, 6, 2},   // left
	{0, 3, 7},   // right
	{0, 7, 4}    // right
};

static r32 global_cube_tex_coords[6][2] = {
   {0.000000, 0.000000},
   {0.000000, 1.000000},
   {0.000000, 0.000000},
   {0.000000, 1.000000},
   {1.000000, 0.000000},
   {1.000000, 1.000000} 
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
	Vec3		world_pos;
	Vec3		orientation;
	Vec3		velocity;
	r32      scale;
};

extern void InitEntities(Platform *pf);
extern void UpdateEntities(GameState *gs, Renderer *ren, Input *in, r32 dt, int num_frames);
extern void RenderEntities(GameState *gs, Renderer *ren);
#endif	// Header guard
