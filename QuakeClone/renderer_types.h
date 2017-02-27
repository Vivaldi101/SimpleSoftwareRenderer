#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include "shared.h"

// based on a vertex list
struct Poly {
	Vec3 *		vertex_list;	// points to the mesh containing the poly

	int			state;
	//int			attr;
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

struct VertexGroup {
	Vec3	vert_array[85];		// cacheline friendly
};

struct PolyGroup {
	Poly	poly_array[64];		// cacheline friendly
};

struct MeshData {
	VertexGroup	*	local_verts;		// original
	VertexGroup	* 	trans_verts;		// transformed

	PolyGroup *		polys;
};
// not self contained
// general poly mesh
struct MeshObject {
	struct {
		int		id;
		char	name[32];

		int		state;
		int		attr;

		r32		avg_radius;
		r32		max_radius;

		r32		world_matrix[4][4];	// currently unused
		Vec3	world_pos;
		Vec3	dir;
		Vec3	ux, uy, uz;

		int		num_verts;
		int		num_polys;
	} status;
	// FIXME: maybe remove the un-named struct status

	MeshData *	mesh;
};

// master poly list
struct RenderList {
	int		state;
	int		attr;
	int		num_polys;

	// FIXME: array constants into macros, 

	/*LinkedPoly *	poly_ptrs[256];*/
	// could also use the poly_ptrs instead
	int			poly_indices[256];
	LinkedPoly	poly_data[256];
};

enum EntityType {
	RT_POLY,
	RT_SPRITE,
	RT_PORTALSURFACE,		// doesn't draw anything, just info for portals

	RT_MAX_REF_ENTITY_TYPE
};

#endif	// Header guard
