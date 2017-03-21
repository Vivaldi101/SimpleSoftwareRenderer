#include "render_queue.h"
#include "renderer.h"

RenderQueue *AllocateRenderQueue(MemoryStack *ms, size_t max_buffer_size) {
	RenderQueue *result = PushStruct(ms, RenderQueue);
	result->max_buffer_size = max_buffer_size;
	result->buffer_base = PushSize(ms, byte, max_buffer_size);

	return result;
}

void ExecuteRenderQueue(RenderQueue *rq, Renderer *ren) {
	RenderCommandHeader *header;

	for (size_t i = 0; i < rq->used_buffer_size;) {
		header = (RenderCommandHeader *)(rq->buffer_base + i);

		if (header->type == RCMD_ClearScreen) {
			ClearScreen *clear = (ClearScreen *)header;
			i += sizeof(ClearScreen);

			R_BeginFrame(ren, 40);
		} else if (header->type == RCMD_ShowScreen) {
			ShowScreen *clear = (ShowScreen *)header;
			i += sizeof(ShowScreen);

			R_EndFrame(ren);
		} else {
			InvalidCodePath;
		}
	}

	rq->used_buffer_size = 0;
}

RenderCommandHeader *_PlaceRenderCommand_(RenderQueue *rq, size_t element_size, RenderCommandEnum type) {
	RenderCommandHeader *result = 0;

	if (rq->used_buffer_size + element_size <= rq->max_buffer_size) {
		result = (RenderCommandHeader *)(rq->buffer_base + rq->used_buffer_size);
		if (type == RCMD_ShowScreen) {
		}
		result->type = type;

		rq->used_buffer_size += element_size;
	} else {
		InvalidCodePath;
	}

	return result;
}