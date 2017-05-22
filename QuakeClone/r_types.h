#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include "shared.h"

struct Bitmap {
	Vec2i	dim;
	byte *	data;		
};

struct Poly {
	Vec3 *		vertex_array;	
	u16			vert_indices[3];
	int			num_verts;
	int			state;
	u32			color;	// rgba, packed
};

struct Vert {
	Vec3	model_space_pos;     
	Vec3	unit_normal;
	u32		color;               // rgba, packed
	r32		u, v;                // texture coordinates
};

//struct PipelineVert {
//	Vec3	view_space_pos;      // xyz
//	Vec3	view_space_normal;   // xyz
//	u8		is_lit;              // a boolean
//	u8		clip_flags;          // six bit flags
//	u16		pad;
//
//	// ready for hardware:
//	Vec4	screen_space_pos;    // xyzw
//	u32		color;               // rgba, packed
//	u32		specular;            // rgba, packed
//	r32		u, v;                // texture coordinates
//}; // 60 bytes


//// self contained
//struct LinkedPoly {
//	Vec3			orig_vertex_array[3];	// original
//	Vec3			trans_vertex_array[3];	// transformed
//
//	int				state;
//	int				attr;
//	u32				color;
//
//	LinkedPoly *	next;
//	LinkedPoly *	prev;
//};
#endif	// Header guard
