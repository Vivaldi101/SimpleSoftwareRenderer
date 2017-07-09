#include "shared.h"

#include "win_r.h"
#include "renderer.h"
#include "r_cmds.h"

Renderer *R_Init(Platform *pf, void *hinstance, void *wndproc) { 
	Renderer *rs = PushStruct(&pf->main_memory_stack.perm_data, Renderer);

	// init renderer backend
	Assert(MAX_NUM_POLYS < 0xffff);
	rs->back_end.polys = PushArray(&pf->main_memory_stack.perm_data, MAX_NUM_POLYS, Poly);
	rs->back_end.poly_verts = PushArray(&pf->main_memory_stack.perm_data, MAX_NUM_POLY_VERTS, PolyVert);
	rs->back_end.lights = PushArray(&pf->main_memory_stack.perm_data, MAX_NUM_LIGHTS, Light);

	// init lights
	Light l = {};
	l.ambient = MakeVec4(1.0f, 1.0f, 1.0f, 1.0f); 
	l.diffuse  = MakeVec4(1.0f, 1.0f, 1.0f, 1.0f); 
	l.specular = MakeVec4(1.0f, 1.0f, 1.0f, 1.0f); 
	l.radius = 10.0f;
	l.kc = 0.0f; 
	l.kl = 0.055f;
	l.kq = 0.0f;
	l.is_active = true;
	l.flags |= (CAMERA_LIGHT|POINT_LIGHT);
	R_AddLight(&rs->back_end, &l);

	// FIXME: this into a separate function
	rs->back_end.cmds.buffer_base = PushSize(&pf->main_memory_stack.perm_data, MAX_RENDER_BUFFER, byte);
	rs->back_end.cmds.max_buffer_size = MAX_RENDER_BUFFER;
	rs->back_end.cmds.used_buffer_size = 0;
	rs->back_end.entities = pf->game_state->entities;

	if (!InitWindow(&rs->back_end.target, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Com_Quit();
	}	

	if (!InitDIB(&rs->back_end.target)) {
		Sys_Print("Error while initializing the DIB\n");
		Com_Quit();
	}

	Sys_Print("Renderer backend init done\n");

	// init renderer frontend
	rs->front_end.is_ambient = AMBIENT_OFF;
	Vec3Init(rs->front_end.current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);

	Vec3Init(rs->front_end.current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
	rs->front_end.current_view.aspect_ratio = (r32)rs->back_end.target.width / (r32)rs->back_end.target.height;

	rs->front_end.current_view.viewplane_width = 2;	
	rs->front_end.current_view.viewplane_height = 2;

	rs->front_end.current_view.fov_y = 90.0f;

	rs->front_end.current_view.z_near = 1.0f;
	rs->front_end.current_view.z_far = 100.0f;

	rs->front_end.current_view.viewport_width = rs->back_end.target.width;		
	rs->front_end.current_view.viewport_height = rs->back_end.target.height;

	//rs->front_end.current_view.world_scale = 1.0f / 1.0f;

	Sys_Print("Renderer frontend init done\n");

	return rs;
}

