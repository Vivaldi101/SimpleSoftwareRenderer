#ifndef QC_WIN_SOFTREND_H
#define QC_WIN_SOFTREND_H
#include "shared.h"
#include "win_local.h"
#include "renderer_local.h"
#include <ddraw.h>

/*
** Windows GDI
*/
b32		DIB_Init(VidSystem *vid_sys);
void	DIB_Shutdown();
void	DIB_SetPalette(const byte *palette);

/*
** Windows Direct Draw
*/
//b32		DDRAW_Init(byte **buffer, int *pitch);
//void	DDRAW_Shutdown();
//void	DDRAW_SetPalette(const byte *palette );


#endif	// Header guard