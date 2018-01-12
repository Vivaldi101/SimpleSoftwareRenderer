#ifndef ENTITY_H
#define ENTITY_H
#include "r_types.h"

#define INVALID_XMACRO_CASE ((EntityEnum)-1)
#define X(name) name,
#define XFunc(name) static void Update##name(void *_data_)

// give here the entity data types to use 
#define XMACRO X(Cube)
#undef X

enum EntityEnum {
#define X(name) name,
	XMACRO
#undef X
};

struct BaseEntity {
	EntityEnum	type;
	int			num_entities;
	void		*data;			// pointer to the specific entity data
};

struct Cube_ {
	Poly 		polys[12];
	PolyVert	local_vertex_array[8];		
	PolyVert	trans_vertex_array[8];		
};

// adds all the entities and returns pointer to the position after the added entities inside entity_buffer
extern byte *CreateEntities(byte *entity_buffer);
#endif	// Header guard
