#ifndef WIN_RENDERER_H
#define WIN_RENDERER_H
#include "shared.h"

extern b32	InitDIB(struct RenderTarget *rt, MemoryStack *ms);
extern void	DIB_Shutdown();

#endif	// Header guard