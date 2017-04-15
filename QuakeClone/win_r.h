#ifndef WIN_RENDERER_H
#define WIN_RENDERER_H
#include "shared.h"
#include "win_local.h"

b32		InitDIB(struct VidSystem *vid_sys);
void	DIB_Shutdown();
//void	DIB_SetPalette(const byte *palette);

#endif	// Header guard