
// print the palette
#if 0
	int c = 0;
	for (int i = 1; i <= 16; ++i) {
		for (int j = 1; j <= 16; ++j) {
			R_DrawRect(rbe->rt, j * 20.0f, i * 20.0f, j * 20.0f + 20.0f, i * 20.0f + 20.0f, c);
			++c;
		}
	}
#endif

// print the colormap
#if 0
void R_DrawColormap(const VidSystem *rt) {
	// FIXME: testing stuff
	extern byte global_colormap[256*64];
	int c = 0;
	for (int i = 1; i <= 64; ++i) {
		for (int j = 1; j <= 256; ++j) {
			R_DrawRect(rt, j * 2.0f, i * 2.0f, j * 2.0f + 50.0f, i * 2.0f + 50.0f, global_colormap[c]);
			++c;
		}
	}
	BitBlt(rt->win_handles.hdc, 0, 0, 800, 600, rt->win_handles.hdc_dib_section, 0, 0, SRCCOPY);
}
#endif