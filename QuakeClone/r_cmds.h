#ifndef RENDERER_CMDS_H
#define RENDERER_CMDS_H
#include "common.h"

enum RenderCommandEnum {
	RCMD_CLEAR,
	RCMD_SWAP_BUFFERS,
	RCMD_BITMAP,
	RCMD_MESH,
	RCMD_END_OF_CMDS
};

struct RenderBasis {
	Vec3	axis[3];			
	Vec3	origin;
};

struct RenderCommands {
	byte *	buffer_base;
	size_t	max_buffer_size;
	size_t	used_buffer_size;
};

struct DrawBitmapCmd {
	int		cmd_id;
	byte *	buffer;		
	u32		color;		
	Dim2d	d2;
};

struct DrawPolyListCmd {
	int		cmd_id;
	byte *	buffer;		
	Poly *	polys;
	Vec3 *	poly_verts;
	int		num_polys;
	u32		pitch;		
	int		bpp;		
	int		width;          // FIXME: pass a 2d dim
	int		height;
	b32		is_wireframe;
};

struct SwapBuffersCmd {
	int		cmd_id;
	HDC		hdc;
	HDC		hdc_dib_section;	
	int		width;          // FIXME: pass a 2d dim
	int		height;
};

struct ClearBufferCmd {
	int		cmd_id;
	byte *	buffer;		
	size_t	size;
};

extern void R_IssueRenderCommands(RenderCommands *rc);
#endif	// Header guard
