#ifndef QC_WIN_SHARED_H
#define QC_WIN_SHARED_H
#include "win_local.h"

struct WinHandles {
	HDC			dc;
	HDC			hdc_dib_section;	// DC compatible with DIB section
	HWND		window;
	HBITMAP		dib_section;		// DIB section
};

#define	MAX_NUM_ARGVS 128

#endif	// Header guard
