#ifndef RENDERER_TYPES_H
#define RENDERER_TYPES_H

#include "shared.h"

struct PolyVert {
	Vec3		xyz;
	r32		w;
	r32		uv[2];
	Vec3		normal;
	//byte		color[4];
};

struct Poly {
	PolyVert *	vertex_array;	
	u16			vert_indices[3];
	int			num_verts;
	int			state;
	u32			color;	// rgba, packed
};

struct Point2D {
	int	x, y;
};



//struct DrawVert {
//	Vec3		xyz;
//	r32			uv[2];
//	r32			lightmap[2];
//	Vec3		normal;
//	byte		color[4];
//};
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


#endif	// Header guard
