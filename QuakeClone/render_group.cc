#include "render_group.h"

RenderGroup *AllocateRenderGroup(MemoryStack *ms, size_t max_buffer_size) {
	RenderGroup *result = PushStruct(ms, RenderGroup);
	result->max_buffer_size = max_buffer_size;
	result->buffer_base = PushSize(ms, byte, max_buffer_size);

	return result;
}

void *PushRenderElement(RenderGroup *rg, size_t element_size) {
	Assert(rg);
	void *result = 0;

	if (rg->used_buffer_size + element_size <= rg->max_buffer_size) {
		result = rg->buffer_base + rg->used_buffer_size;
		rg->used_buffer_size += element_size;
	} else {
		InvalidCodePath;
	}

	return result;
}