#include "render_queue.h"
#include "renderer.h"

RenderCommands *AllocateRenderCommands(MemoryStack *ms, size_t max_buffer_size) {
	RenderCommands *result = PushStruct(ms, RenderCommands);
	result->max_buffer_size = max_buffer_size;
	result->buffer_base = PushSize(ms, byte, max_buffer_size);
	result->used_buffer_size = 0;

	return result;
}

void ExecuteRenderCommands(RenderCommands *rc, Renderer *ren) {
	Assert(rc->used_buffer_size > 0);
	RenderCommandHeader *header;

	for (size_t i = 0; i < rc->used_buffer_size;) {
		header = (RenderCommandHeader *)(rc->buffer_base + i);

		if (header->type == RCMD_ClearScreen) {
			ClearScreen *clear = (ClearScreen *)header;
			i += sizeof(ClearScreen);

			R_BeginFrame(ren, (byte)clear->fill_color);
		} else if (header->type == RCMD_ShowScreen) {
			ShowScreen *show = (ShowScreen *)header;
			i += sizeof(ShowScreen);

			R_EndFrame(ren);
		} else {
			InvalidCodePath("Invalid renderer command type");
		}
	}

	rc->used_buffer_size = 0;
}

void _ApplyRenderCommand_(RenderCommands *rc, RenderCommandEnum type, size_t element_size, char *format, ...) {
	if (rc->used_buffer_size + element_size <= rc->max_buffer_size) {
		RenderCommandHeader *rch = (RenderCommandHeader *)(rc->buffer_base + rc->used_buffer_size);
		rch->type = type;
		char *start, *p = format, *q = (char *)rch;

		VaStart(start, format);

		for (; *p; ++p) {
			if (*p == '%') {
				switch (*(p + 1)) {
					case 'd': {
						int offset = VaArg(start, int);
						Assert(offset >= 0);
						int data = VaArg(start, int);
						*(int *)(q + offset) = data;
					} break;
					case 'f': {
						int offset = VaArg(start, int);
						Assert(offset >= 0);
						r64 data = VaArg(start, r64);
						*(r32 *)(q + offset) = (r32)data;
					} break;

					default: InvalidCodePath("Unrecognized formatter");
				}
			}
		}
	} else {
		InvalidCodePath("Renderer buffer overflow");
	}
}

//void _EndRenderCommand_(RenderCommands *rc, size_t element_size) {
//	rc->used_buffer_size += element_size;
//}
