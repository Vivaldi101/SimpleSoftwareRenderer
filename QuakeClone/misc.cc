
// print the palette
#if 0
	int c = 0;
	for (int i = 1; i <= 16; ++i) {
		for (int j = 1; j <= 16; ++j) {
			R_DrawRect(rbe->vid_sys, j * 20.0f, i * 20.0f, j * 20.0f + 20.0f, i * 20.0f + 20.0f, c);
			++c;
		}
	}
#endif