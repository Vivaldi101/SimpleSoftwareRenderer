#ifndef RENDERER_CMDS_H
#define RENDERER_CMDS_H
#include "common.h"

enum RenderCommandEnum {
	RCMD_CLEAR,
	RCMD_SWAP_BUFFERS,
	RCMD_RECT,
	RCMD_MESH,
	RCMD_END_OF_CMDS
};

enum RectTypeEnum {
	RECT_FONT
};

struct RenderBasis3d {
	Vec3	axis[3];			
	Vec3	origin;
};

struct RenderBasis2d {
	Vec2	axis[2];			
	Vec2	origin;
};

struct RenderCommands {
	byte *	buffer_base;
	size_t	max_buffer_size;
	size_t	used_buffer_size;
};

struct DrawRectCmd {
	int				cmd_id;
	RenderBasis2d	basis;
	byte *			data;		
	u32				color;		
	Dim2d			d2;
	RectTypeEnum	type;
};

struct DrawPolyListCmd {
	int		cmd_id;
	Poly *	polys;
	Vec3 *	poly_verts;
	int		num_polys;
	b32		is_wireframe;	
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
#endif	// Header guard
