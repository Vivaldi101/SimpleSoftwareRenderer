#ifndef RENDERER_GROUP_H
#define RENDERER_GROUP_H
#include "common.h"

#define RCMD(type) RCMD_##type
struct RenderCommands {
	byte *			buffer_base;
	size_t			max_buffer_size;
	size_t			used_buffer_size;
};

enum RenderCommandEnum {
	RCMD_ClearScreen,
	RCMD_ShowScreen,
	RCMD_Poly
};

struct RenderCommandHeader {
	RenderCommandEnum	type;
};

struct ClearScreen {
	RenderCommandHeader	header;
	u32 fill_color;
};

struct ShowScreen {
	RenderCommandHeader	header;
	int	width, height;
};

extern RenderCommands *AllocateRenderCommands(MemoryStack *ms, size_t max_buffer_size);
extern void ExecuteRenderCommands(RenderCommands *rc, Renderer *ren);

#define ApplyRenderCommand(rc, render_type, format, ...)_ApplyRenderCommand_(rc, RCMD_##render_type, sizeof(render_type), format, __VA_ARGS__)
extern void _ApplyRenderCommand_(RenderCommands *rc, RenderCommandEnum type, size_t element_size, char *format, ...);

#define EndRenderCommand(rc, render_type) { (rc)->used_buffer_size += sizeof(render_type); } 

//extern void _EndRenderCommand_(RenderCommands *rc, size_t element_size);
#endif	// Header guard
