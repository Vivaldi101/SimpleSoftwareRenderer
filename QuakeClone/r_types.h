#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include "shared.h"

struct PolyVert {
	Vec3	v;	
};

// based on a vertex list
struct Poly {
	Vec3 *		vertex_list;	// points to the mesh containing the poly
	int			state;
	u32			color;
	int			vert_indices[3];
};

// self contained
struct LinkedPoly {
	Vec3			orig_vertex_array[3];	// original
	Vec3			trans_vertex_array[3];	// transformed

	int				state;
	int				attr;
	u32				color;

	LinkedPoly *	next;
	LinkedPoly *	prev;
};

// FIXME: remove 
struct VertexGroup {
	Vec3	vert_array[85];		// cacheline friendly
};

struct PolyGroup {
	Poly	poly_array[64];		// cacheline friendly
};

struct Mesh {
	VertexGroup	*	local_verts;		// original
	VertexGroup	* 	trans_verts;		// transformed

	PolyGroup *		polys;
};

#endif	// Header guard
