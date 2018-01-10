#ifndef RENDERER_LOCAL_H
#define RENDERER_LOCAL_H

#include "shared.h"
#include "common.h"
#include "win_shared.h"
#include "r_types.h"
#include "r_cmds.h"
#include "lights.h"

#define PLATFORM_FULLSCREEN
#define BYTES_PER_PIXEL 4	

// FIXME: put into enums
#define POLY_STATE_ACTIVE	0x0001
#define POLY_STATE_CLIPPED	0x0002
#define POLY_STATE_BACKFACE	0x0004
#define POLY_STATE_LIT		0x0008
//#define POLY_STATE_VISIBLE	0x0010

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
	//b32			full_screen;
};

struct ViewSystem {
	r32				view_matrix[4][4];
	r32				projection_matrix[4][4];
	r32				screen_matrix[3][3];

	Orientation		world_orientation;
	Vec3			target;
	Vec3			velocity;

	int				viewport_x,		viewport_y;
	int				viewport_width, viewport_height;
	int				viewplane_width, viewplane_height;

	r32				fov_x, fov_y;
	r32				view_dist;	

	Plane			frustum[NUM_FRUSTUM_PLANES];			// order of left, right, top, bottom, near z
	r32				z_far, z_near;

	r32				aspect_ratio;
	//r32				world_scale;
};

struct RendererFrontend {
	ViewSystem		current_view;	
	b32				is_wireframe;
	// for fun
	AmbientState	is_ambient;
};

struct RendererBackend {
	RenderCommands		cmds;
	RenderTarget 		target;		

	Entity *			entities;

	Light *				lights;
	int					num_lights;

	Poly *				polys;			
	PolyVert *			poly_verts;		
	int					num_polys;
	int					num_poly_verts;
};

struct Renderer {
	RendererFrontend	front_end;
	RendererBackend		back_end;
};

extern void R_Init(Renderer **ren, void *hinstance, void *wndproc); 
extern void RF_RenderView(ViewSystem *vs);
extern void RF_SetupFrustum(ViewSystem *vs);
extern void RF_SetupProjection(ViewSystem *vs);

extern void RF_AddPolys(RendererBackend *rb, const PolyVert *verts, Poly *poly_array, int num_polys);

extern void RF_TransformModelToWorld(PolyVert *local_poly_verts, PolyVert *trans_poly_verts, int num_verts, Vec3 world_pos, r32 world_scale);
extern void RF_TransformWorldToView(ViewSystem *vs, PolyVert *poly_verts, int num_verts);
extern void RF_TransformViewToClip(ViewSystem *vs, PolyVert *poly_verts, int num_verts);
extern void RF_TransformClipToScreen(ViewSystem *vs, PolyVert *poly_verts, int num_verts);

ClipFlags RF_CullPointAndRadius(ViewSystem *vs, Vec3 pt, r32 radius = 1.0f);
extern void RF_CullBackFaces(ViewSystem *vs, Poly *polys, int num_polys);

extern void RF_CalculateVertexNormals(Poly *polys, int num_polys, PolyVert *poly_verts, int num_poly_verts);


extern void RB_ExecuteRenderCommands(RenderTarget *rt, const void *data);


#endif	// Header guard
