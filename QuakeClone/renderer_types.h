#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

// based on a vertex list
struct Poly {
	Vec3 *		vertex_list;	// points to the mesh containing the poly

	int			state;
	int			attr;
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

// not self contained
// general poly mesh
struct MeshData {
	int		id;
	char	name[64];

	int		state;
	int		attr;

	r32		avg_radius;
	r32		max_radius;

	Vec3	world_pos;
	Vec3	dir;
	Vec3	ux, uy, uz;

	int		num_verts;
	int		num_polys;

	// FIXME: array constants into macros, 
	// and the lists with proper memory management
	 
	Vec3	local_vertex_array[120];	// original
	Vec3	trans_vertex_array[120];	// transformed

	Poly	poly_array[40];
};

// master poly list
struct RenderList {
	int		state;
	int		attr;
	int		num_polys;

	// FIXME: array constants into macros, 
	// and the lists with proper memory management

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

struct RefDef {
	int			x, y, width, height;
	float		fov_x, fov_y;
	Vec3		view_org;
	Vec3		view_axis[3];		// transformation matrix
};

#endif	// Header guard
