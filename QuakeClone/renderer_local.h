#ifndef RENDERER_LOCAL_H
#define RENDERER_LOCAL_H

#include "shared.h"
#include "common.h"
#include "renderer_types.h"
#include "win_shared.h"

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

// max values

#define	MAX_POLYS		600
#define	MAX_POLYVERTS	3000

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
	//FRUSTUM_PLANE_FAR,
	//FRUSTUM_PLANE_NEAR,

	NUM_FRUSTUM_PLANES
};


struct VidSystem {
	byte *			buffer;		
	int				pitch;		
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
	r32				screen_matrix[4][4];

	Orientation		world_orientation;
	//Orientation		debug_orientation;
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

struct BackEnd {
	//drawSurf_t	drawSurfs[MAX_DRAWSURFS];
	//dlight_t	dlights[MAX_DLIGHTS];

	// FIXME: change MeshObject to Entity
	MeshObject 	entities[MAX_ENTITIES];
	int			num_entities;
	Poly *		polys;			//[MAX_POLYS];
	Vec3 *		poly_verts;		//[MAX_POLYVERTS];
	//struct MeshData *	mesh_data;	// fixed num of verts and polys
	//renderCommandList_t	commands;
};

struct Renderer {
	struct RenderQueue *	queue;
	BackEnd *				back_end;
	VidSystem 				vid_sys;
	ViewSystem				current_view;	
};


//extern unsigned global_8to24able[256]; 


extern void R_Init(Platform *pf, void *hinstance, void *wndproc); 

extern void R_GenerateDrawSurfs(Renderer *ren);
extern void R_RenderView(Renderer *ren);

extern void R_BeginFrame(Renderer *ren, byte fill_color);
extern void R_EndFrame(Renderer *ren);

extern void R_SetupFrustum(Renderer *ren);
//extern void R_SetupEulerView(Renderer *ren, r32 pitch, r32 yaw, r32 roll, r32 view_orig_x, r32 view_orig_y, r32 view_orig_z);
extern void R_SetupProjection(Renderer *ren);

extern void R_TransformModelToWorld(Renderer *ren, MeshObject *md, VertexTransformState ts = VTS_LOCAL_TO_TRANSFORMED);
extern void R_TransformWorldToView(Renderer *ren, MeshObject *md);
extern void R_TransformViewToClip(Renderer *ren, MeshObject *md);
extern void R_TransformClipToScreen(Renderer *ren, MeshObject *md);

extern void R_DrawWireframeMesh(Renderer *ren, MeshObject *md);
extern void R_DrawSolidMesh(Renderer *ren, MeshObject *md);

extern void R_RotatePoints(r32 rot_mat[3][3], Vec3 *points, int num_verts);
FrustumClippingState R_CullPointAndRadius(Renderer *ren, Vec3 pt, r32 radius = 0.0f);
extern void R_CullBackFaces(Renderer *ren, MeshObject *md);


#endif	// Header guard
