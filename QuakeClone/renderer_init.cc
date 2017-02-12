#include "win_renderer.h"
#include "renderer_local.h"

void R_Init(EngineData *ed, void *hinstance, void *wndproc) { 
	ed->renderer = (Renderer *)Allocate(ed->stack_allocator, sizeof(Renderer));
	ed->renderer->back_end = (BackEnd *)Allocate(ed->stack_allocator, sizeof(BackEnd));
	ed->renderer->back_end->polys = (Poly *)Allocate(ed->stack_allocator, sizeof(Poly) * MAX_POLYS);

	if (!Vid_CreateWindow(ed->renderer, WINDOW_WIDTH, WINDOW_HEIGHT, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Sys_Quit();
	}	

	if (!DIB_Init(&ed->renderer->vid_sys)) {
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
