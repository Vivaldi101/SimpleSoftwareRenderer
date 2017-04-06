#ifndef RENDERER_CMDS_H
#define RENDERER_CMDS_H
#include "common.h"

enum RenderCommandEnum {
	RCMD_CLEAR,
	RCMD_SWAP_BUFFERS,
	RCMD_MESH,
	RCMD_END_OF_CMDS
};

struct RenderCommands {
	byte *	buffer_base;
	size_t	max_buffer_size;
	size_t	used_buffer_size;
};

struct DrawPolyListCmd {
	int		cmd_id;
	byte *	buffer;		
	Poly *	polys;
	Vec3 *	poly_verts;
	int		num_polys;
	u32		pitch;		
	int		bpp;		
	u32		color;		
	int		width;          
	int		height;
	b32		is_wireframe;
};

struct SwapBuffersCmd {
	int		cmd_id;
	HDC		hdc;
	HDC		hdc_dib_section;	
	int		width;          
	int		height;
};

struct ClearBufferCmd {
	int		cmd_id;
	byte *	buffer;		
	u32		pitch;		
	int		height;
};

extern void R_IssueRenderCommands(RenderCommands *rc);
#endif	// Header guard
