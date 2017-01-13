#ifndef QC_SOFTR_LOCAL_H
#define QC_SOFTR_LOCAL_H

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
	LOCAL_ONLY = 0,
	TRANSFORMED_ONLY,
	LOCAL_TO_TRANSFORMED
};

enum FrustumClippingState {
	CULL_IN = 0,	// completely unclipped
	CULL_CLIP,		// clipped by one or more planes
	CULL_OUT		// completely outside the clipping planes
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
	r32				world_view_matrix[4][4];
	//Vec4			view_perspective_matrix[4];
	//Vec4			perspective_screen_matrix[4];

	Orientation		world_orientation;
	Orientation		debug_orientation;
	Vec3			target;

	r32				viewport_x,		viewport_y;
	r32				viewport_width, viewport_height;
	r32				viewplane_width, viewplane_height;

	r32				fov_h, fov_v;
	r32				view_dist_h, view_dist_v;	// FIXME: maybe only need the view_dist_h, rename it to just "view_dist"


	Plane			frustum[4];			// order of left, right, top, bottom, FIXME: add near and far z
	r32				z_far, z_near;

	r32				aspect_ratio;

	int				state, attr;
};

struct RendererState {
	VidSystem 		vid_sys;
	ViewSystem		current_view;	
};
extern RendererState global_renderer_state;


extern unsigned global_8to24able[256]; 


void R_Init(void *hinstance, void *wndproc); 
void R_RenderView(/*viewParms_t *parms*/);

void R_EndFrame();
void R_BeginFrame(MeshObject *md);

void R_SetupFrustum(r32 fov_h, r32 z_near, r32 z_far);
void R_SetupEulerView(r32 pitch, r32 yaw, r32 roll, r32 view_orig_x, r32 view_orig_y, r32 view_orig_z);

void R_TransformModelToWorld(MeshObject *md, VertexTransformState ts = LOCAL_TO_TRANSFORMED);
void R_TransformWorldToView(MeshObject *md);
void R_TransformViewToClip(MeshObject *md);
void R_TransformClipToScreen(MeshObject *md);
void R_DrawMesh(MeshObject *md);
void R_DrawGradient(VidSystem *vid_sys);

void R_RotatePoints(Vec3 (*rot_mat)[3], Vec3 *points, int num_points);
FrustumClippingState R_CullPointAndRadius(Vec3 pt, r32 radius = 0.0f);
void R_CullBackFaces(MeshObject *md);


#endif	// Header guard
