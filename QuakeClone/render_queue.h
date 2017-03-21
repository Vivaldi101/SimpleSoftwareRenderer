#ifndef RENDERER_GROUP_H
#define RENDERER_GROUP_H
#include "common.h"

struct RenderQueue {
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
	int					width, height;
	r32					r, g, b, a;
};

struct ShowScreen {
	RenderCommandHeader	header;
	int					width, height;
};

extern RenderQueue *AllocateRenderQueue(MemoryStack *ms, size_t max_buffer_size);
extern void ExecuteRenderQueue(RenderQueue *rq, Renderer *ren);

#define PlaceRenderCommand(group, type)(type *)_PlaceRenderCommand_(group, sizeof(type), RCMD_##type)
extern RenderCommandHeader *_PlaceRenderCommand_(RenderQueue *rq, size_t element_size, RenderCommandEnum type);
#endif	// Header guard
