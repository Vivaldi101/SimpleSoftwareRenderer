#include "shared.h"

#include "win_renderer.h"
#include "renderer.h"
#include "render_queue.h"

Renderer *R_Init(MemoryStack *mem_stack, void *hinstance, void *wndproc) { 
	Renderer *ren = PushStruct(mem_stack, Renderer);
	ren->polys = PushArray(mem_stack, MAX_POLYS, Poly);
	ren->commands = AllocateRenderCommands(mem_stack, MEGABYTES(4));

	if (!Vid_CreateWindow(ren, WINDOW_WIDTH, WINDOW_HEIGHT, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Sys_Quit();
	}	

	if (!DIB_Init(&ren->vid_sys)) {
		Sys_Print("Error while initializing the DIB\n");
		Sys_Quit();
	}

	Sys_Print("Renderer init done\n");

	return ren;
}
