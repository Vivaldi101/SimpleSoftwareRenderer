#ifndef WIN_RENDERER_H
#define WIN_RENDERER_H
#include "shared.h"
#include "win_local.h"

/*
** Windows GDI
*/
b32		DIB_Init(struct VidSystem *vid_sys);
void	DIB_Shutdown();
void	DIB_SetPalette(const byte *palette);

#endif	// Header guard