#ifndef RENDERER_GROUP_H
#define RENDERER_GROUP_H
#include "common.h"

struct RenderQueue {
	byte *			buffer_base;
	size_t			max_buffer_size;
	size_t			used_buffer_size;
};

enum RenderCommandEnum {
	Command_ClearScreen,
	Command_Poly
};

struct RenderCommandHeader {
	RenderCommandEnum	type;
};

struct ClearScreen {
	RenderCommandHeader	header;
	r32					r, g, b, a;
};

extern RenderQueue *AllocateRenderQueue(MemoryStack *ms, size_t max_buffer_size);
extern void ExecuteRenderQueue(RenderQueue *rq, Renderer *ren);

#define PlaceRenderCommand(group, type)(type *)_PlaceRenderCommand_(group, sizeof(type), Command_##type)
extern RenderCommandHeader *_PlaceRenderCommand_(RenderQueue *rq, size_t element_size, RenderCommandEnum type);
#endif	// Header guard
