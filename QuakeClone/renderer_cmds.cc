#include "shared.h"
#include "renderer_local.h"

void R_BeginFrame(Renderer *ren, byte fill_color) {
	// clean up the framebuffer
	void *buffer = ren->vid_sys.buffer;
	int pitch = ren->vid_sys.pitch;
	int height = ren->vid_sys.height;
	memset(buffer, fill_color, pitch * height);
}

void R_EndFrame(Renderer *ren) {
	if (!ren->vid_sys.state.full_screen) {
		BitBlt(ren->vid_sys.win_handles.dc,
			   0, 0,
			   ren->vid_sys.width,
			   ren->vid_sys.height,
			   ren->vid_sys.win_handles.hdc_dib_section,
			   0, 0, SRCCOPY);
	}
}


