#include "shared.h"
#include "win_r.h"
#include "renderer.h"
#include "r_cmds.h"

void R_Init(Renderer *ren, Platform *pf, void *hinstance, void *wndproc) { 
	// init some test lights
	Light l = {};
	l.ambient = MV4(0.20f, 0.20f, 0.20f, 1.0f); 
	l.diffuse  = MV4(0.25f, 0.0f, 0.0f, 1.0f); 
	l.specular = MV4(0.0f, 0.0f, 0.5f, 0.0f); 
	l.radius = 1.0f;
	l.kc = 0.0f; 
	l.kl = 0.05000f;
	l.kq = 0.0f;
	l.is_active = true;
	l.flags |= (CAMERA_LIGHT|POINT_LIGHT);
	R_AddLight(&ren->back_end, &l);

	if (!InitWindow(&ren->back_end.target, wndproc, hinstance)) {
		Sys_Print("Error while creating the window\n");
		Com_Quit();
	}	

	if (!InitDIB(&ren->back_end.target, &pf->main_memory_stack.perm_data)) {
		Sys_Print("Error while initializing the DIB\n");
		Com_Quit();
	}

	Sys_Print("Renderer backend init done\n");

	ren->front_end.is_ambient = AMBIENT_ON;
	Vec3Init(ren->front_end.current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);

	Vec3Init(ren->front_end.current_view.world_orientation.dir, 0.0f, 0.0f, 1.0f);
	ren->front_end.current_view.aspect_ratio = (r32)ren->back_end.target.width / (r32)ren->back_end.target.height;

	ren->front_end.current_view.viewplane_width = 2;	
	ren->front_end.current_view.viewplane_height = 2;

	ren->front_end.current_view.fov_y = 90.0f;

	ren->front_end.current_view.z_near = 1.0f;
	ren->front_end.current_view.z_far = 1000.0f;

	ren->front_end.current_view.viewport_width = ren->back_end.target.width;		
	ren->front_end.current_view.viewport_height = ren->back_end.target.height;

	Sys_Print("Renderer frontend init done\n");
}

