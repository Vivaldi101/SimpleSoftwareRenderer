#include "renderer_local.h"
#include "win_renderer.h"
#include "plg_loader.h"

unsigned global_8to24able[256];
RendererState global_renderer_state;

#if 1
void R_DrawGradient(VidSystem *vid_sys) {
	u32 width		= vid_sys->width;
	u32 height		= vid_sys->height;
	u32 pitch		= vid_sys->pitch;
	byte* row		= vid_sys->buffer;

	for (u32 i = 0; i < height; ++i) {
		u32* pixel = (u32*)row;
		*pixel = 0;
		for (u32 j	= 0; j < width; ++j) {
			byte G		= (byte)i;
			byte B		= (byte)j;
			*pixel++	= (G << 16 | B);
		}
		row += pitch;
	}
}
#endif

b32 R_Init(void *hinstance, void *wndproc) {	// FIXME: Rendering functions into own .dll

	memset(&global_renderer_state, 0, sizeof(RendererState));
	RendererState *rs = &global_renderer_state;
	// should be 1920 / 2, 1080 / 2
	Vid_CreateWindow(rs, 1920, 1080, wndproc, hinstance);	

	if (!DIB_Init(&rs->vid_sys)) {
		Sys_Print("Error while initializing the DIB");
		Sys_Quit();
	}


	int x = 42;

	return true;
} 

#if 1
void R_SetupFrustum(r32 fov_h, r32 z_near, r32 z_far) {
	ViewSystem *vs = &global_renderer_state.current_view;
	Vec3 view_dir = global_renderer_state.current_view.world_orientation.dir;
	Vec3 origin = vs->world_orientation.origin;

	vs->z_near = z_near;
	vs->z_far = z_far;

	vs->aspect_ratio = (r32)global_renderer_state.vid_sys.width / (r32)global_renderer_state.vid_sys.height;

	vs->viewplane_width = 2.0f;	// normalized
	vs->viewplane_height = vs->viewplane_width / vs->aspect_ratio;
	vs->viewport_width = (r32)global_renderer_state.vid_sys.width;		// conversions for now
	vs->viewport_height = (r32)global_renderer_state.vid_sys.height;	// --,,--

	vs->fov_h = fov_h;


	r32 half_tan_h = tan(DEG2RAD(fov_h / 2.0f));

	vs->view_dist_h = (vs->viewplane_width / 2.0f) / half_tan_h;
	vs->view_dist_v = (vs->viewplane_height / 2.0f) / half_tan_h;
	vs->fov_v = RAD2DEG(atan((vs->viewplane_height / 2.0f) / vs->view_dist_h)) * 2.0f;


	if (origin[2] != 0.0f) {
		if (view_dir[1] != 0.0f) {
			int x = 42;
		}
	}

	// cos and -sin of left clipping plane
	r32 yaw = DEG2RAD(view_dir[1] - (vs->fov_h * 0.5f));

	// left plane normal
	Vec3 lpn = {};
	//lpn[0] = (origin[0] - vs->viewplane_width / 2.0f);
	//lpn[0] *= (-sin(yaw));
	//lpn[2] = (origin[2] - vs->view_dist_h);
	//lpn[2] *= (cos(yaw));

	Vector3Init(lpn, (-sin(yaw) * -(vs->viewplane_width)) / 2.0f, 0, (cos(yaw) * vs->view_dist_h)); 
	PerpOperator(lpn, 0, 2);
	vs->frustum_planes[FRUSTUM_PLANE_INDEX_LEFT].unit_normal = lpn;

	// cos and sin of right clipping plane
	yaw = DEG2RAD(view_dir[1] + (vs->fov_h * 0.5f));

	// right plane normal
	Vec3 rpn = {};
	//rpn[0] = (origin[0] + vs->viewplane_width / 2.0f);
	//rpn[0] *= (sin(yaw));
	//rpn[2] = (origin[2] - vs->view_dist_h);
	//rpn[2] *= (cos(yaw));

	Vector3Init(rpn, (sin(yaw) * vs->viewplane_width) / 2.0f, 0, (cos(yaw) * vs->view_dist_h)); 
	PerpOperator(rpn, 2, 0);
	vs->frustum_planes[FRUSTUM_PLANE_INDEX_RIGHT].unit_normal = rpn;


	// FIXME: top and bottom plane normals
	// top plane normal
	Vec3 tpn;
	Vector3Init(tpn, 0, vs->viewplane_height / 2.0f, vs->view_dist_h);
	PerpOperator(tpn, 2, 1);
	vs->frustum_planes[FRUSTUM_PLANE_INDEX_TOP].unit_normal = tpn;

	// bottom plane normal
	Vec3 bpn;
	Vector3Init(bpn, 0, -(vs->viewplane_height) / 2.0f, vs->view_dist_h);
	PerpOperator(bpn, 1, 2);
	vs->frustum_planes[FRUSTUM_PLANE_INDEX_BOTTOM].unit_normal = bpn;

	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		vs->frustum_planes[i].unit_normal = Vector3Normalize(&vs->frustum_planes[i].unit_normal);
		vs->frustum_planes[i].dist = Vector3DotProduct(vs->frustum_planes[i].unit_normal, vs->world_orientation.origin);
		//if (vs->frustum_planes[i].dist < 0.0f) {
		//	int x = 42;
		//}
		vs->frustum_planes[i].type = PLANE_NON_AXIAL;
	}

	int x = 42;
	
}
#endif

#if 0
void R_SetupFrustum (r32 fov_h, r32 z_near, r32 z_far) {
	int		i;
	float	xs, xc;
	float	ang;

	ang = global_renderer_state.current_view.fov_h / 180 * M_PI * 0.5f;
	xs = sin(ang);
	xc = cos(ang);

	Vector3Scale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[0].normal );
	Vector3MA( tr.viewParms.frustum[0].normal, xc, tr.viewParms.or.axis[1], tr.viewParms.frustum[0].normal );

	Vector3Scale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[1].normal );
	Vector3MA( tr.viewParms.frustum[1].normal, -xc, tr.viewParms.or.axis[1], tr.viewParms.frustum[1].normal );

	ang = global_renderer_state.current_view.fov_v / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	Vector3Scale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[2].normal );
	Vector3MA( tr.viewParms.frustum[2].normal, xc, tr.viewParms.or.axis[2], tr.viewParms.frustum[2].normal );

	Vector3Scale( tr.viewParms.or.axis[0], xs, tr.viewParms.frustum[3].normal );
	Vector3MA( tr.viewParms.frustum[3].normal, -xc, tr.viewParms.or.axis[2], tr.viewParms.frustum[3].normal );

	for (i = 0; i < NUM_FRUSTUM_PLANES; i++) {
		tr.viewParms.frustum[i].type = PLANE_NON_AXIAL;
		tr.viewParms.frustum[i].dist = Vector3DotProduct(tr.viewParms.or.origin, tr.viewParms.frustum[i].normal);
		//SetPlaneSignbits( &tr.viewParms.frustum[i] );
	}
}
#endif


void R_SetupEulerView(r32 pitch, r32 yaw, r32 roll, r32 view_orig_x, r32 view_orig_y, r32 view_orig_z) {
	Vec4 inv_trans_mat[4]; 
	Vec4 final[4];

	Vec4 rot_x[4], rot_y[4], rot_z[4];
	Vec4 tmp1[4];
	Vec4 tmp2[4];

	Vec4 tmp3[4];
	Vec4 tmp4[4];

	ViewSystem *vs = &global_renderer_state.current_view;
	Matrix4x4SetIdentity(vs->world_view_matrix);
	Matrix4x4SetIdentity(vs->view_perspective_matrix);
	Matrix4x4SetIdentity(vs->perspective_screen_matrix);
	Vector3Init(vs->world_orientation.origin, view_orig_x, view_orig_y, view_orig_z);
	Vector3Copy(vs->debug_orientation.origin, vs->world_orientation.origin);
	Vec3 origin;
	Vector3Copy(origin, vs->world_orientation.origin);
	Vector3Negate(origin);
	Matrix3x3SetIdentity(global_renderer_state.current_view.world_orientation.axis);


	Matrix4x4SetIdentity(inv_trans_mat);
	Matrix4x4SetIdentity(rot_x);
	Matrix4x4SetIdentity(rot_y);
	Matrix4x4SetIdentity(rot_z);

	Matrix4x4SetIdentity(tmp1);
	Matrix4x4SetIdentity(tmp2);
	Matrix4x4SetIdentity(tmp3);
	Matrix4x4SetIdentity(tmp4);

	// inverse translation matrix for the view
	Vector3Copy(inv_trans_mat[3], origin);

	// get the euler angles in degrees
	r32 theta_x = vs->world_orientation.dir.v.x = pitch;
	r32 theta_y = vs->world_orientation.dir.v.y = yaw;
	r32 theta_z = vs->world_orientation.dir.v.z = roll;

	// inverse rotation matrix around the x axis
	r32 cos_theta = cos(DEG2RAD(theta_x));
	r32 sin_theta = -sin(DEG2RAD(theta_x));

	Vector3Init(rot_x[1], 0, cos_theta, sin_theta); 
	Vector3Init(rot_x[2], 0, -sin_theta, cos_theta); 

	// inverse rotation matrix around the y axis
	cos_theta = cos(DEG2RAD(theta_y));
	sin_theta = -sin(DEG2RAD(theta_y));

	Vector3Init(rot_y[0], cos_theta, 0, -sin_theta); 
	Vector3Init(rot_y[2], sin_theta, 0, cos_theta); 

	// inverse rotation matrix around the z axis
	cos_theta = cos(DEG2RAD(theta_z));
	sin_theta = -sin(DEG2RAD(theta_z));

	Vector3Init(rot_z[0], cos_theta, sin_theta, 0); 
	Vector3Init(rot_z[1], -sin_theta, cos_theta, 0); 

	//// FIXME: move the matrix products into own routine
	//// euler pitch, yaw, roll order
	//// testing some funny shit
	//Negate(rot_y[0].v.z);
	//Negate(rot_y[2].v.x);

	//for (int i = 0; i < 3; ++i) {
	//	tmp1[i][0] = Vector3DotProduct(rot_x[i], rot_y[0]);
	//	tmp1[i][1] = Vector3DotProduct(rot_x[i], rot_y[1]);
	//	tmp1[i][2] = Vector3DotProduct(rot_x[i], rot_y[2]);
	//}

	//Negate(rot_z[0].v.y);
	//Negate(rot_z[1].v.x);

	//for (int i = 0; i < 3; ++i) {
	//	tmp2[i][0] = Vector3DotProduct(tmp1[i], rot_z[0]);
	//	tmp2[i][1] = Vector3DotProduct(tmp1[i], rot_z[1]);
	//	tmp2[i][2] = Vector3DotProduct(tmp1[i], rot_z[2]);
	//}

	//Vector3Copy(tmp2[3], origin);
	//memcpy(vs->world_view_matrix, &tmp2, sizeof(tmp2));


	//Negate(rot_y[0].v.z);
	//Negate(rot_y[2].v.x);

	//Negate(rot_z[0].v.y);
	//Negate(rot_z[1].v.x);

	MatrixMultiply(&inv_trans_mat, &rot_x, &tmp3);
	MatrixMultiply(&tmp3, &rot_y, &tmp4);
	MatrixMultiply(&tmp4, &rot_z, &final);
	memcpy(vs->world_view_matrix, &final, sizeof(final));
}

// FIXME: maybe use 1x4 by 4x4 matrix multiply
// memcpy all the verts to a temp buffer and transform
// with an invisible homogeneous 1
void R_TransformWorldToView(MeshData *md) {
	if (!(md->state & CULL_OUT)) {
		int num_verts = md->num_verts;
		for (int i = 0; i < num_verts; ++i) {
			Vec4 vert, tmp;
			Vector3Copy(vert, md->trans_vertex_array[i]);
			vert.v.w = 1;

			MatrixMultiply(&vert, &global_renderer_state.current_view.world_view_matrix, &tmp);  
			Vector3Copy(md->trans_vertex_array[i], tmp);
		}
	} else {
		int x = 42;
	}
}

void R_TransformModelToWorld(MeshData *md, VertexTransformState ts) {
	int num_verts = md->num_verts;

	if (ts == LOCAL_TO_TRANSFORMED) {
		for (int i = 0; i < num_verts; ++i) {
			Vector3Add(md->local_vertex_array[i], md->world_pos, md->trans_vertex_array[i]);
		}
	} else if (ts == TRANSFORMED_ONLY) {
		for (int i = 0; i < num_verts; ++i) {
			Vector3Add(md->trans_vertex_array[i], md->world_pos, md->trans_vertex_array[i]);
		}
	}
}

FrustumClippingState R_CullPointAndRadius(Vec3 pt, r32 radius) {
	FrustumClippingState cull_state[NUM_FRUSTUM_PLANES] = {CULL_IN, CULL_IN, CULL_IN, CULL_IN};
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		Plane pl = global_renderer_state.current_view.frustum_planes[i];

		r32 dist = Vector3DotProduct(pt, pl.unit_normal) - pl.dist;
		if (dist > radius) {
			//Sys_Print("Point center is outside of a plane\n");
			//if (dot - radius > 0.0f) {
			//	Sys_Print("Point with radius is outside of the frustum\n");
			//	return CULL_OUT;
			//} else {
			//	Sys_Print("Point with radius is partially clipped of the frustum\n");
			//	return CULL_CLIP;
			//}
			//Sys_Print("Point is culled!\n");
			//return CULL_IN;
			cull_state[i] = CULL_OUT;
		}  
	}

	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		if (cull_state[i] == CULL_OUT) {
			Sys_Print("Point is culled!!\n");
			return CULL_OUT;
		}
	}

	Sys_Print("NO culling!!\n");
	return CULL_IN;
	//Sys_Print("Point is on a plane of the frustum\n");
	//return CULL_OUT;
}

void R_TransformViewToClip(MeshData *md) {
	if (!(md->state & CULL_OUT)) {
		int num_verts = md->num_verts;

		r32 dist = global_renderer_state.current_view.view_dist_h;
		r32 z;
		for (int i = 0; i < num_verts; ++i) {
			z = md->trans_vertex_array[i].v.z;
			md->trans_vertex_array[i].v.x = (md->trans_vertex_array[i].v.x * dist) / z; 
			md->trans_vertex_array[i].v.y = (md->trans_vertex_array[i].v.y * dist) / z; 
		}
	}
}

void R_TransformClipToScreen(MeshData *md) {
	if (!(md->state & CULL_OUT)) {
		int num_verts = md->num_verts;

		r32 screen_width_factor = (0.5f * global_renderer_state.current_view.viewport_width) - 0.5f;
		r32 screen_height_factor = (0.5f * global_renderer_state.current_view.viewport_height) - 0.5f;
		for (int i = 0; i < num_verts; ++i) {
			md->trans_vertex_array[i].v.x = screen_width_factor + (md->trans_vertex_array[i].v.x * screen_width_factor);
			md->trans_vertex_array[i].v.y = screen_height_factor - (md->trans_vertex_array[i].v.y * screen_height_factor);
		}
	}
}

// Cohen-Sutherland clipping constants
#define INSIDE	 0	
#define LEFT	 1	
#define RIGHT	 2	
#define BOTTOM	 4	
#define TOP		 8  
static int R_GetLineClipCode(int x, int y) {
	int code = INSIDE;												// initialised as being inside of [[clip window]]

	if (x < 0) {													// to the left of clip window
		code |= LEFT;
	} else if (x >= global_renderer_state.vid_sys.width) {			// to the right of clip window
		code |= RIGHT;
	}
	if (y < 0) {													// below the clip window
		code |= BOTTOM;
	} else if (y >= global_renderer_state.vid_sys.height) {			// above the clip window
		code |= TOP;
	}

	return code;
}

// Cohen–Sutherland 
// FIXME: try Liang–Barsky or even Cyrus–Beck 
static void R_DrawLine(int x0, int y0, int x1, int y1, u32 color) {
	int outcode0	= R_GetLineClipCode(x0, y0);
	int outcode1	= R_GetLineClipCode(x1, y1);
	b32 accept		= false;

	for (;;) {
		if (!(outcode0 | outcode1)) {		// Trivially accept 
			accept = true;
			break;
		} else if (outcode0 & outcode1) {	// Trivially reject 
			break;
		} else {
			// Casting the operands to reals so div by zero wont cause exeption, only inf
			r32 x;
			r32 y;
			r32 m = (r32)(y1 - y0) / (r32)(x1 - x0);

			// Clip the line segment(s) laying outside of the screen
			// Pick the one that is outside of the clipping rect
			int chosen_code = outcode0 ? outcode0 : outcode1;

			if (chosen_code & TOP) {           // point is above the clip rectangle
				y = (r32)global_renderer_state.vid_sys.height - 1.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & BOTTOM) { // point is below the clip rectangle
				y = 0.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & RIGHT) {  // point is to the right of clip rectangle
				x = (r32)global_renderer_state.vid_sys.width - 1.0f;
				y = m * (x - x0) + y0;
			} else if (chosen_code & LEFT) {   // point is to the left of clip rectangle
				x = 0.0f;
				y = m * (x - x0) + y0;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (chosen_code == outcode0) {
				x0 = (int)x;
				y0 = (int)y;
				outcode0 = R_GetLineClipCode(x0, y0);
			} else {
				x1 = (int)x;
				y1 = (int)y;
				outcode1 = R_GetLineClipCode(x1, y1);
			}
		}
	}
	if (accept) {
		// Bresenham
		// FIXME: improve on this
		int dx			= abs(x1 - x0);
		int dy			= abs(y1 - y0);
		int pitch		= global_renderer_state.vid_sys.pitch / global_renderer_state.vid_sys.bpp;
		int numerator	= 0;

		int denominator;	
		int num_pixels;
		int add;

		int x1_inc;		// If 1 increment every iteration, 0 if not
		int x2_inc;		// Incremented only when numerator >= denominator

		int y1_inc;		// If pitch increment every iteration, 0 if not
		int y2_inc;		// Incremented only when numerator >= denominator

		if (x0 <= x1) {	// left to right
			x1_inc = 1;
			x2_inc = 1;
		} else {		// right to left
			x1_inc = -1;
			x2_inc = -1;
		}
		if (y0 <= y1) {	// top to bottom (in screen)
			y1_inc = pitch;
			y2_inc = pitch;
		} else {		// bottom to top
			y1_inc = -pitch;
			y2_inc = -pitch;
		}

		// X- or Y-dominate line
		if (dx >= dy) {
			x2_inc = 0;
			y1_inc = 0;
			denominator = dx;
			num_pixels	= dx;
			add			= dy;
		} else {
			x1_inc = 0;
			y2_inc = 0;
			denominator = dy;
			num_pixels	= dy;
			add			= dx;
		}
		numerator = denominator / 2;
		u32 *line = (u32*)global_renderer_state.vid_sys.buffer;
		line = (line + (pitch * y0)) + x0;

		for (int i = 0; i < num_pixels; ++i) {
			*line = color;
			numerator += add;
			if (numerator >= denominator) {
				numerator -= denominator;
				line += x2_inc;	// Inc x in case of Y-dominant line
				line += y2_inc; // Inc y in case of X-dominant line
			}
			line += x1_inc;
			line += y1_inc;
		}
	}
}

void R_DrawMesh(MeshData *md) {
	if (!(md->state & CULL_OUT)) {
		int num_polys = md->num_polys;

		for (int i = 0; i < num_polys; ++i) {
			if ((md->poly_array[i].state & POLY_STATE_BACKFACE)) {
				continue;
			}

			int v0 = md->poly_array[i].vert_indices[0];
			int v1 = md->poly_array[i].vert_indices[1];
			int v2 = md->poly_array[i].vert_indices[2];

			R_DrawLine((int)md->trans_vertex_array[v0].v.x,
					   (int)md->trans_vertex_array[v0].v.y,
					   (int)md->trans_vertex_array[v1].v.x,
					   (int)md->trans_vertex_array[v1].v.y,
					   md->poly_array[i].color);

			R_DrawLine((int)md->trans_vertex_array[v1].v.x,
					   (int)md->trans_vertex_array[v1].v.y,
					   (int)md->trans_vertex_array[v2].v.x,
					   (int)md->trans_vertex_array[v2].v.y,
					   md->poly_array[i].color);

			R_DrawLine((int)md->trans_vertex_array[v2].v.x,
					   (int)md->trans_vertex_array[v2].v.y,
					   (int)md->trans_vertex_array[v0].v.x,
					   (int)md->trans_vertex_array[v0].v.y,
					   md->poly_array[i].color);

			//R_DrawLine((int)global_renderer_state.current_view.debug_orientation.origin[0],
			//		   (int)global_renderer_state.current_view.debug_orientation.origin[1], 
			//		   (int)md->trans_vertex_array[v1][0], (int)md->trans_vertex_array[v1][1], 0xff);
		}


	}
}

void R_RotatePoints(Vec3 (*rot_mat)[3], Vec3 *points, int num_points) {
	for (int i = 0; i < num_points; ++i) {
		r32 x = Vector3DotProduct((*rot_mat)[0], points[i]);
		r32 y = Vector3DotProduct((*rot_mat)[1], points[i]);
		r32 z = Vector3DotProduct((*rot_mat)[2], points[i]);

		points[i][0] = x;
		points[i][1] = y;
		points[i][2] = z;
	}
}

void R_CullBackFaces(MeshData *md) {
	if (!(md->state & CULL_OUT)) {

		int num_polys = md->num_polys;
		for (int i = 0; i < num_polys; ++i) {
			if ((md->poly_array[i].state & POLY_STATE_BACKFACE) || !(md->poly_array[i].state & POLY_STATE_ACTIVE)) {
				continue;
			}

			int v0 = md->poly_array[i].vert_indices[0];
			int v1 = md->poly_array[i].vert_indices[1];
			int v2 = md->poly_array[i].vert_indices[2];

			Vec3 u = Vector3Build(md->trans_vertex_array[v0], md->trans_vertex_array[v1]);
			Vec3 v = Vector3Build(md->trans_vertex_array[v0], md->trans_vertex_array[v2]);
			Vec3 n = CrossProduct(u, v);
			Vec3 view = Vector3Build(global_renderer_state.current_view.world_orientation.origin, md->trans_vertex_array[v0]);

			if (global_renderer_state.current_view.world_orientation.origin[2] > 0.0f) {
				int x = 42;
			}

			r32 dot = Vector3DotProduct(view, n);

			if (dot <= 0.0f) {
				md->poly_array[i].state |= POLY_STATE_BACKFACE;
			}
		}
	}
}
