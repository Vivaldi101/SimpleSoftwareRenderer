#ifndef RENDERER_LOCAL_H
#define RENDERER_LOCAL_H

#include "shared.h"
#include "common.h"
#include "win_shared.h"
#include "r_types.h"
#include "r_cmds.h"
#include "lights.h"

#define BYTES_PER_PIXEL 4
// FIXME: maybe move all the constants these into .cc file if only impl needs access

// FIXME: put into enums
#define POLY_STATE_ACTIVE	0x0001
#define POLY_STATE_CLIPPED	0x0002
#define POLY_STATE_BACKFACE	0x0004
#define POLY_STATE_VISIBLE	0x0008

// FIXME: put into enums
#define POLY_ATTR_2SIDED		0x0001
#define POLY_ATTR_TRANSPARENT	0x0002
#define POLY_ATTR_8BITCOLOR		0x0004
#define POLY_ATTR_RGB16			0x0008
#define POLY_ATTR_RGB24			0x0010

// FIXME: put into enums
#define POLY_ATTR_SHADE_MODE_PURE		0x0020
#define POLY_ATTR_SHADE_MODE_FLAT		0x0040
#define POLY_ATTR_SHADE_MODE_GOURAUD	0x0080
#define POLY_ATTR_SHADE_MODE_PHONG		0x0100

enum ClipFlags {
	CULL_IN,		// completely unclipped
	CULL_CLIP,		// clipped by one or more planes
	CULL_OUT		// completely outside the clipping planes
};

enum { 
	FRUSTUM_PLANE_LEFT, 
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_NEAR,
	//FRUSTUM_PLANE_FAR,

	NUM_FRUSTUM_PLANES
};

struct RenderTarget {
	byte *			buffer;		
	int				pitch;		
	int				width;          
	int				height;
	int				bpp;

	WinHandles 		win_handles;
	//b32				full_screen;
};

struct ViewSystem {
	r32				view_matrix[4][4];
	r32				projection_matrix[4][4];
	r32				screen_matrix[4][4];

	Orientation		world_orientation;
	Vec3			target;
	Vec3			velocity;

	int				viewport_x,		viewport_y;
	int				viewport_width, viewport_height;
	int				viewplane_width, viewplane_height;

	r32				fov_x, fov_y;
	r32				view_dist;	

	Plane			frustum[NUM_FRUSTUM_PLANES];			// order of left, right, top, bottom, FIXME: far z
	r32				z_far, z_near;

	r32				aspect_ratio;
};

struct RendererFrontend {
	ViewSystem		current_view;	
	b32				is_wireframe;
	// for fun
	AmbientState	is_ambient;
};

struct RendererBackend {
	RenderCommands		cmds;
	RenderTarget *		target;

	Entity *			entities;

	Light *				lights;
	int					num_lights;

	Poly *				polys;			
	Vec3 *				poly_verts;		// FIXME: change type to proper vert type 			
	int					num_polys;
	int					num_verts;
};

struct RenderingSystem {
	RendererFrontend	front_end;
	RendererBackend		back_end;
};

//
//	Renderer frontend
//
extern RenderingSystem *R_Init(Platform *pf, void *hinstance, void *wndproc); 

extern void R_BeginFrame(RenderTarget *rt, RenderCommands *rc);
extern void R_EndFrame(RenderTarget *rt, RenderCommands *rc);
extern void R_PushRectCmd(RenderTarget *rt, RenderCommands *rc, Dim2d d2, Vec4 color, Vec2 origin);
extern void R_PushPolysCmd(RenderTarget *rt, RenderCommands *rc, Poly *polys, Vec3 *poly_verts, int num_polys, b32 solid);

extern void R_RenderView(ViewSystem *vs);

extern void R_SetupFrustum(ViewSystem *vs);
extern void R_SetupProjection(ViewSystem *vs);

extern void R_TransformModelToWorld(Vec3 *local_poly_verts, Vec3 *trans_poly_verts, int num_verts, Vec3 world_pos);
extern void R_TransformWorldToView(ViewSystem *vs, Vec3 *poly_verts, int num_verts);
extern void R_TransformViewToClip(ViewSystem *vs, Vec3 *poly_verts, int num_verts);
extern void R_TransformClipToScreen(ViewSystem *vs, Vec3 *poly_verts, int num_verts);

extern void R_RotatePoints(r32 rot_mat[3][3], Vec3 *points, int num_verts);
ClipFlags R_CullPointAndRadius(ViewSystem *vs, Vec3 pt, r32 radius = 1.0f);
extern void R_CullBackFaces(ViewSystem *vs, Poly *polys, const Vec3 *poly_verts, int num_polys);
extern void R_AddPolys(RendererBackend *rb, const Vec3 *verts, Poly *poly_array, int num_polys);


//
//	Renderer backend
//
extern void RB_ExecuteRenderCommands(RenderTarget *rt, const void *data);
extern void R_DrawRect(RenderTarget *rt, r32 rmin_x, r32 rmin_y, 
					   r32 rmax_x, r32 rmax_y,
					   byte color);


#endif	// Header guard
