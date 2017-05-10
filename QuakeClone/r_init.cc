#include "shared.h"

#include "win_r.h"
#include "renderer.h"
#include "r_cmds.h"

RenderingSystem *R_Init(Platform *pf, void *hinstance, void *wndproc) { 
	RenderingSystem *rs = PushStruct(&pf->main_memory_stack.perm_data, RenderingSystem);

	// init backend
	rs->back_end.target = PushStruct(&pf->main_memory_stack.perm_data, RenderTarget);

	Assert(MAX_NUM_POLYS < 0xffff);
	rs->back_end.polys = PushArray(&pf->main_memory_stack.perm_data, MAX_NUM_POLYS, Poly);
	rs->back_end.poly_verts = PushArray(&pf->main_memory_stack.perm_data, MAX_NUM_POLY_VERTS, Vec3);
	rs->back_end.lights = PushArray(&pf->main_memory_stack.perm_data, MAX_NUM_LIGHTS, Light);
	R_AddLight(&rs->back_end, MakeVec4(1.0f, 1.0f, 1.0f, 1.0f), MakeVec4(1.0f, 1.0f, 1.0f, 1.0f), MakeVec4(1.0f, 1.0f, 1.0f, 1.0f), MakeVec3(0.0f, 0.0f, 0.0f), 10.0f, 0.0f, 0.0055f, 0.0f, (LightTypeFlags)(CAMERA_LIGHT|SPOT_LIGHT));

	// FIXME: this into a separate function
	rs->back_end.cmds.buffer_base = PushSize(&pf->main_memory_stack.perm_data, MAX_RENDER_BUFFER, byte);
	rs->back_end.cmds.max_buffer_size = MAX_RENDER_BUFFER;
	rs->back_end.cmds.used_buffer_size = 0;
	rs->back_end.entities = pf->game_state->entities;

	if (!InitWindow(rs->back_end.target, WINDOW_WIDTH, WINDOW_HEIGHT, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Com_Quit();
	}	

	if (!InitDIB(rs->back_end.target)) {
		Sys_Print("Error while initializing the DIB\n");
		Com_Quit();
	}

	Sys_Print("Renderer backend init done\n");

	// init frontend
	rs->front_end.is_ambient = AMBIENT_ON;
	Vec3Init(rs->front_end.current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);

	Vec3Init(rs->front_end.current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
	rs->front_end.current_view.aspect_ratio = (r32)rs->back_end.target->width / (r32)rs->back_end.target->height;

	// FIXME: handle non-homogeneous viewplanes
	rs->front_end.current_view.viewplane_width = 2;	// normalized viewplane
	rs->front_end.current_view.viewplane_height = 2;

	rs->front_end.current_view.fov_y = 90.0f;

	rs->front_end.current_view.z_near = 30.0f;
	rs->front_end.current_view.z_far = 1000.0f;

	rs->front_end.current_view.viewport_width = rs->back_end.target->width;		
	rs->front_end.current_view.viewport_height = rs->back_end.target->height;

	Sys_Print("Renderer frontend init done\n");

	return rs;
}

