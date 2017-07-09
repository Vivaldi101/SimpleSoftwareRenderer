#ifndef RENDERER_CMDS_H
#define RENDERER_CMDS_H
#include "common.h"

enum RenderCommandEnum {
	RCMD_CLEAR = 1,
	RCMD_SWAP_BUFFERS,
	RCMD_RECT,
	RCMD_TEXT,
	RCMD_MESH,
	RCMD_END_OF_CMDS
};

struct Basis3d {
	Vec3	axis[3];	// FIXME: matrix types
	Vec3	origin;
};

struct Basis2d {
	Vec2	axis[2];	// FIXME: matrix types	
	Vec2	origin;
};

struct RenderCommands {
	byte *	buffer_base;
	size_t	max_buffer_size;
	size_t	used_buffer_size;
};

struct DrawRectCmd {
	int				cmd_id;
	Basis2d			basis;
	Bitmap			bitmap;
	//Vec2			points[4];	
	Vec2i			dim;	
	u32				color;		// rgba, packed
};

struct DrawTextCmd {
	int				cmd_id;
	Basis2d			basis;
	Bitmap *		bitmap;
	Vec2i			dim;	
	const char *	text;
	int				gap;
	u32				color;		// rgba, packed
};

struct DrawPolyCmd {
	int			cmd_id;
	Poly *		polys;
	PolyVert *	poly_verts;
	int			num_polys;
	b32			is_wireframe;	
};

struct SwapBuffersCmd {
	int		cmd_id;
	HDC		hdc;
	HDC		hdc_dib_section;	
};

struct ClearBufferCmd {
	int		cmd_id;
	size_t	size;
};

extern void R_IssueRenderCommands(struct RenderTarget *rt, RenderCommands *rc);
extern void R_BeginFrame(RenderTarget *rt, RenderCommands *rc);
extern void R_EndFrame(RenderTarget *rt, RenderCommands *rc);
extern void R_PushRectCmd(RenderCommands *rc, Bitmap bm, Vec2 origin, r32 scale = 1.0f, Vec4 color = MakeVec4(1.0f,1.0f,1.0f,1.0f));
extern void R_PushTextCmd(RenderCommands *rc, const char *text, Bitmap *bm, Vec2 origin, r32 scale = 1.0f, Vec4 color = MakeVec4(1.0f,1.0f,1.0f,1.0f));
extern void R_PushPolysCmd(RenderCommands *rc, Poly *polys, PolyVert *poly_verts, int num_polys, b32 is_wireframe);
#endif	// Header guard
