#ifndef RENDERER_LOCAL_H
#define RENDERER_LOCAL_H

#include "shared.h"
#include "common.h"
#include "win_shared.h"
#include "r_types.h"
#include "r_cmds.h"
#include "lights.h"

//#define PLATFORM_FULLSCREEN
#define BYTES_PER_PIXEL 4	
#define DEPTH_BUFFER_DIM 2048

enum {
	POLY_STATE_CULLED = 0,		
	POLY_STATE_CLIPPED = 1,		
	//POLY_STATE_LIT = 1 << 1,		
	POLY_STATE_BACKFACE = 1 << 2		
};

enum {
	CULL_IN,		// polygon completely unclipped
	CULL_CLIP,	// polygon clipped by one or more planes
	CULL_OUT		// polygon completely outside the clipping planes
};

enum { 
	FRUSTUM_PLANE_LEFT, 
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_TOP,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_NEAR,

	NUM_FRUSTUM_PLANES
};

struct RenderTarget {
	byte *			frame_buffer;		
	r32 		      depth_buffer[DEPTH_BUFFER_DIM][DEPTH_BUFFER_DIM];  // FIXME: variable size?
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

	Plane			   frustum[NUM_FRUSTUM_PLANES];			// world space order of left, right, top, bottom, near z
	Plane			   view_planes[NUM_FRUSTUM_PLANES];    // view space order of left, right, top, bottom, near z
	r32				z_near, z_far;

	r32				aspect_ratio;
	//r32				world_scale;
};

struct RendererFrontend {
	ViewSystem		current_view;	
	AmbientState	is_ambient;
};

struct RendererBackend {
	RenderCommands			cmds;
	RenderTarget 			target;		

	Bitmap               test_texture;     // FIXME: remove
	Light *					lights;
	int						num_lights;

	Poly *					polys;			   
	Poly *					vis_polys;			
	PolyVert *				poly_verts;		
	PolyVert *				vis_poly_verts;		
	int						num_polys;			      // num polys total
	int						num_poly_verts;		   // num verts total
	int						vis_num_polys;			   // num polys in a frame (subset of total)
	int						vis_num_poly_verts;		// num verts in a frame (subset of total)
};

struct Renderer {
	RendererFrontend	front_end;
	RendererBackend		back_end;
};

extern void R_Init(Renderer **ren, MemoryStack *ms, void *hinstance, void *wndproc); 
extern void RF_UpdateView(ViewSystem *vs);
extern void RF_ExtractWorldPlanes(ViewSystem *vs);
extern void RF_ExtractViewPlanes(ViewSystem *vs);
extern void RF_SetupProjection(ViewSystem *vs);

extern void RF_AddCubePolys(RendererBackend *rb, ViewSystem *vs, const PolyVert *verts, const u16 (*index_list)[3], int num_polys, r32 texture_scale);
extern void RF_AddPolys(RendererBackend *rb, const PolyVert *verts, const u16 (*index_list)[3], int num_polys);

extern void RF_TransformModelToWorld(const PolyVert *local_poly_verts, PolyVert *trans_poly_verts, int num_verts, Vec3 world_pos, r32 world_scale);
extern void RF_TransformWorldToView(ViewSystem *vs, PolyVert *poly_verts, int num_verts);
extern void RF_TransformViewToProj(ViewSystem *vs, PolyVert *poly_verts, int num_verts);
extern void RF_TransformProjToClip(PolyVert *poly_verts, int num_verts);
extern void RF_Clip(Renderer *r);
extern void RF_TransformClipToScreen(ViewSystem *vs, PolyVert *poly_verts, int num_verts);

extern int RF_CullPointAndRadius(ViewSystem *vs, Vec3 pt, r32 radius);
extern int RF_CullPoly(ViewSystem *vs, Poly *poly);
extern void RF_CullBackFaces(ViewSystem *vs, Poly *polys, int num_polys);

extern void RF_CalculateVertexNormals(Poly *polys, int num_polys, PolyVert *poly_verts, int num_poly_verts);

extern void RB_ExecuteRenderCommands(RenderTarget *rt, const void *data);


#endif	// Header guard
