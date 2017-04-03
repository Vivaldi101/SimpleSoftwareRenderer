#include "shared.h"

#include "win_r.h"
#include "renderer.h"
#include "r_cmds.h"

RenderingSystem *R_Init(const Platform *pf, void *hinstance, void *wndproc) { 
	RenderingSystem *rs = PushStruct(pf->main_memory_stack.perm_data, RenderingSystem);

	// init backend
	rs->back_end.vid_sys = PushStruct(pf->main_memory_stack.perm_data, VidSystem);
	rs->back_end.polys = PushArray(pf->main_memory_stack.perm_data, MAX_NUM_POLYS, Poly);
	rs->back_end.poly_verts = PushArray(pf->main_memory_stack.perm_data, MAX_NUM_POLY_VERTS, Vec3);
	Assert(MAX_NUM_POLYS < 0xffff);

	// FIXME: this into a proper function
	const int render_buffer_size = MEGABYTES(4);
	rs->back_end.cmds.buffer_base = PushSize(pf->main_memory_stack.perm_data, render_buffer_size, byte);
	rs->back_end.cmds.max_buffer_size = render_buffer_size;
	rs->back_end.cmds.used_buffer_size = 0;
	rs->back_end.entities = pf->game_state->entities;

	if (!Vid_CreateWindow(rs->back_end.vid_sys, WINDOW_WIDTH, WINDOW_HEIGHT, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Sys_Quit();
	}	

	if (!DIB_Init(rs->back_end.vid_sys)) {
		Sys_Print("Error while initializing the DIB\n");
		Sys_Quit();
	}

	Sys_Print("Renderer backend init done\n");

	// init frontend
	Vec3Init(rs->front_end.current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);

	Vec3Init(rs->front_end.current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
	rs->front_end.current_view.aspect_ratio = (r32)rs->back_end.vid_sys->width / (r32)rs->back_end.vid_sys->height;

	// FIXME: handle non-homogeneous viewplanes
	rs->front_end.current_view.viewplane_width = 2;	// normalized viewplane
	rs->front_end.current_view.viewplane_height = 2;

	rs->front_end.current_view.fov_y = 90.0f;

	rs->front_end.current_view.z_near = 50.0f;
	rs->front_end.current_view.z_far = 500.0f;

	rs->front_end.current_view.viewport_width = (u32)rs->back_end.vid_sys->width;		
	rs->front_end.current_view.viewport_height = (u32)rs->back_end.vid_sys->height;

	Sys_Print("Renderer frontend init done\n");

	return rs;
}

