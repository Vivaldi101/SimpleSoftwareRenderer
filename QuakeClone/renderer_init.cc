#include "shared.h"

#include "win_renderer.h"
#include "renderer_local.h"
#include "render_group.h"

void R_Init(Platform *pf, void *hinstance, void *wndproc) { 
	pf->renderer = PushStruct(pf->perm_data, Renderer);
	pf->renderer->back_end = PushStruct(pf->perm_data, BackEnd);
	pf->renderer->back_end->polys = PushArray(pf->perm_data, MAX_POLYS, Poly);

	AllocateRenderGroup(pf->temp_data, MEGABYTES(4));
	

	if (!Vid_CreateWindow(pf->renderer, WINDOW_WIDTH, WINDOW_HEIGHT, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Sys_Quit();
	}	

	if (!DIB_Init(&pf->renderer->vid_sys)) {
		Sys_Print("Error while initializing the DIB\n");
		Sys_Quit();
	}

	Sys_Print("Renderer init done\n");
	//
	// init function tables
	//
	//for ( i = 0; i < FUNCTABLE_SIZE; i++ )
	//{
	//	tr.sinTable[i]		= sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
	//	tr.squareTable[i]	= ( i < FUNCTABLE_SIZE/2 ) ? 1.0f : -1.0f;
	//	tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
	//	tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

	//	if ( i < FUNCTABLE_SIZE / 2 )
	//	{
	//		if ( i < FUNCTABLE_SIZE / 4 )
	//		{
	//			tr.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
	//		}
	//		else
	//		{
	//			tr.triangleTable[i] = 1.0f - tr.triangleTable[i-FUNCTABLE_SIZE / 4];
	//		}
	//	}
	//	else
	//	{
	//		tr.triangleTable[i] = -tr.triangleTable[i-FUNCTABLE_SIZE/2];
	//	}
	//}
}
