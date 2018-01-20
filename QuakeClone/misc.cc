#include "shared.h"

#if 0
// print the palette
	int c = 0;
	for (int i = 1; i <= 16; ++i) {
		for (int j = 1; j <= 16; ++j) {
			R_DrawRect(rbe->rt, j * 20.0f, i * 20.0f, j * 20.0f + 20.0f, i * 20.0f + 20.0f, c);
			++c;
		}
	}

// print the colormap
void draw_bmp(byte *buffer, int w, int h) {
	BitBlt(rt->win_handles.hdc, 0, 0, w, h, rt->win_handles.hdc_dib_section, 0, 0, SRCCOPY);
}
#endif


