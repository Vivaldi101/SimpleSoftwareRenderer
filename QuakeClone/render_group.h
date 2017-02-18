#ifndef RENDERER_GROUP_H
#define RENDERER_GROUP_H
#include "common.h"

struct RenderGroup {
	Platform *		platform;

	size_t			max_buffer_size;
	size_t			used_buffer_size;

	byte *			buffer_base;
};

RenderGroup *AllocateRenderGroup(MemoryStack *ms, size_t max_buffer_size);
#endif	// Header guard
