#ifndef RENDERER_LOCAL_H
#define RENDERER_LOCAL_H

#include "shared.h"
#include "renderer_types.h"
#include "win_shared.h"


/*
**	POLYGON CONSTANTS
*/

// FIXME: maybe move all the constants these into .cc file if only impl needs access

// states of polygons and faces
#define POLY_STATE_ACTIVE	0x0001
#define POLY_STATE_CLIPPED	0x0002
#define POLY_STATE_BACKFACE	0x0004
#define POLY_STATE_VISIBLE	0x0008

// attributes of polygons and polygon faces
#define POLY_ATTR_2SIDED		0x0001
#define POLY_ATTRRANSPARENT		0x0002
#define POLY_ATTR_8BITCOLOR		0x0004
#define POLY_ATTR_RGB16			0x0008
#define POLY_ATTR_RGB24			0x0010

#define POLY_ATTR_SHADE_MODE_PURE		0x0020
#define POLY_ATTR_SHADE_MODE_FLAT		0x0040
#define POLY_ATTR_SHADE_MODE_GOURAUD	0x0080
#define POLY_ATTR_SHADE_MODE_PHONG		0x0100

/*
**	END OF POLYGON CONSTANTS
*/

enum VertexTransformState {
	VTS_LOCAL_ONLY = 0,
	VTS_TRANSFORMED_ONLY,
	VTS_LOCAL_TO_TRANSFORMED
};

enum FrustumClippingState {
	FCS_CULL_IN = 0,	// completely unclipped
	FCS_CULL_CLIP,		// clipped by one or more planes
	FCS_CULL_OUT		// completely outside the clipping planes
};

enum { 
	FRUSTUM_PLANE_LEFT = 0, 
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,

	NUM_FRUSTUM_PLANES
};


struct VidSystem {
	byte *			buffer;		// invisible buffer
	int				pitch;		// may be > width if displayed in a window
	int				width;          
	int				height;
	int				bpp;

	WinHandles 		win_handles;
	union {
		b32 full_screen;
	} state;

	//byte *			color_map;	// 256 * VID_GRADES size
	//byte *			alpha_map;	// 256 * 256 translucency map
};

struct ViewSystem {
	r32				view_matrix[4][4];
	r32				projection_matrix[4][4];
	//Vec4			perspective_screen_matrix[4];

	Orientation		world_orientation;
	Orientation		debug_orientation;
	Vec3			target;

	r32				viewport_x,		viewport_y;
	r32				viewport_width, viewport_height;
	r32				viewplane_width, viewplane_height;

	r32				fov_x, fov_y;
	r32				view_dist;	


	Plane			frustum[4];			// order of left, right, top, bottom, FIXME: add near and far z
	r32				z_far, z_near;

	r32				aspect_ratio;

	int				state, attr;
};

struct Renderer {
	VidSystem 		vid_sys;
	ViewSystem		current_view;	
};
//extern Renderer *global_renderer;


//extern unsigned global_8to24able[256]; 


extern void R_InitRenderer(Renderer *ren, void *hinstance, void *wndproc); 
extern void R_RenderView();

extern void R_EndFrame();
extern void R_BeginFrame();

extern void R_SetupFrustum();
extern void R_SetupEulerView(r32 pitch, r32 yaw, r32 roll, r32 view_orig_x, r32 view_orig_y, r32 view_orig_z);
extern void R_SetupProjection();

extern void R_TransformModelToWorld(MeshObject *md, VertexTransformState ts = VTS_LOCAL_TO_TRANSFORMED);
extern void R_TransformWorldToView(MeshObject *md);
extern void R_TransformViewToClip(MeshObject *md);
extern void R_TransformClipToScreen(MeshObject *md);
extern void R_DrawMesh(MeshObject *md);
extern void R_DrawGradient(VidSystem *vid_sys);

extern void R_RotatePoints(Vec3 (*rot_mat)[3], Vec3 *points, int num_points);
FrustumClippingState R_CullPointAndRadius(Vec3 pt, r32 radius = 0.0f);
extern void R_CullBackFaces(MeshObject *md);


#endif	// Header guard
