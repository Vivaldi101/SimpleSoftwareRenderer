#include "renderer_local.h"
#include "win_renderer.h"
#include "plg_loader.h"

void R_SetupProjection(Renderer *ren) {
	r32 aspect_ratio = ren->current_view.aspect_ratio;
	r32 fov_y = ren->current_view.fov_y;

	r32 d = (ren->current_view.viewplane_width / 2.0f) / tan(DEG2RAD(fov_y / 2.0f));
	// direct3d style [0, 1] z-buffer mapping
	r32 a = ren->current_view.z_far / (ren->current_view.z_far - ren->current_view.z_near);
	r32 b = -ren->current_view.z_near * a;
	r32 m00 = 1.0f / aspect_ratio;
	r32 projection_matrix[16];

	projection_matrix[0] = m00 * d;
	projection_matrix[4] = 0.0f;
	projection_matrix[8] = 0.0f;
	projection_matrix[12] = 0.0f;

	projection_matrix[1] = 0.0f;
	projection_matrix[5] = d;
	projection_matrix[9] = 0.0f;
	projection_matrix[13] = 0.0f;

	projection_matrix[2] = 0.0f;
	projection_matrix[6] = 0.0f;
	projection_matrix[10] = a;
	projection_matrix[14] = b;

	projection_matrix[3] = 0.0f;
	projection_matrix[7] = 0.0f;
	projection_matrix[11] = 1.0f;
	projection_matrix[15] = 0.0f;

	ren->current_view.view_dist = d;
	memcpy(ren->current_view.projection_matrix, projection_matrix, sizeof(projection_matrix));
}

#if 0
void R_SetupFrustum(r32 fov_h, r32 z_near, r32 z_far) {
	Vec3 origin = ren->current_view.world_orientation.origin;
	ren->current_view.z_near = z_near;
	ren->current_view.z_far = z_far;

	ren->current_view.fov_h = fov_h;
	r32 half_tan_h = tan(DEG2RAD(fov_h / 2.0f));

	ren->current_view.view_dist_h = (ren->current_view.viewplane_width / 2.0f) / half_tan_h;
	ren->current_view.view_dist_v = (ren->current_view.viewplane_height / 2.0f) / half_tan_h;
	ren->current_view.fov_v = RAD2DEG(atan((ren->current_view.viewplane_height / 2.0f) / ren->current_view.view_dist_h)) * 2.0f;

	Plane frustum[4];
	Vec3 axis[3];

	memcpy(&axis, &ren->current_view.world_orientation.axis, sizeof(axis));

	r32 ang = DEG2RAD(fov_h * 0.5f);
	r32 xs = sinf(ang);
	r32 xc = cosf(ang);

	Vector3Scale(axis[0], xs, frustum[0].unit_normal);
	Vector3MA(frustum[0].unit_normal, xc, axis[2], frustum[0].unit_normal);

	Vector3Scale(axis[2], xs, frustum[1].unit_normal);
	Vector3MA(frustum[1].unit_normal, -xc, axis[0], frustum[1].unit_normal);

	ang = DEG2RAD((ren->current_view.fov_v * 0.5f));
	xs = sinf(ang);
	xc = cosf(ang);

	Vector3Scale(axis[1], xs, frustum[2].unit_normal);
	Vector3MA(frustum[2].unit_normal, xc, axis[2], frustum[2].unit_normal);

	Vector3Scale(axis[2], xs, frustum[3].unit_normal);
	Vector3MA(frustum[3].unit_normal, -xc, axis[1], frustum[3].unit_normal);

	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		frustum[i].unit_normal = Vector3Normalize(frustum[i].unit_normal);
		frustum[i].dist = Vector3DotProduct(frustum[i].unit_normal, origin);
		frustum[i].type = PLANE_NON_AXIAL;
	}

	memcpy(&ren->current_view.frustum, &frustum, sizeof(frustum));
}
#endif

#if 1
void R_SetupFrustum(Renderer *ren) {
	r32 fov_x = ren->current_view.fov_x;
	r32 fov_y = ren->current_view.fov_y;
	Vec3 origin = ren->current_view.world_orientation.origin;

	Plane frustum[4];
	Vec3 axis[3];

	memcpy(&axis, &ren->current_view.world_orientation.axis, sizeof(axis));

	r32 ang = DEG2RAD(fov_x * 0.5f);
	r32 xs = sinf(ang);
	r32 xc = cosf(ang);

	Vector3Scale(axis[0], xs, frustum[0].unit_normal);
	Vector3MA(frustum[0].unit_normal, xc, axis[2], frustum[0].unit_normal);

	Vector3Scale(axis[2], xs, frustum[1].unit_normal);
	Vector3MA(frustum[1].unit_normal, -xc, axis[0], frustum[1].unit_normal);

	ang = DEG2RAD(fov_y * 0.5f);
	xs = sinf(ang);
	xc = cosf(ang);

	Vector3Scale(axis[1], xs, frustum[2].unit_normal);
	Vector3MA(frustum[2].unit_normal, xc, axis[2], frustum[2].unit_normal);

	Vector3Scale(axis[2], xs, frustum[3].unit_normal);
	Vector3MA(frustum[3].unit_normal, -xc, axis[1], frustum[3].unit_normal);

	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		frustum[i].unit_normal = Vector3Normalize(frustum[i].unit_normal);
		frustum[i].dist = Vector3DotProduct(frustum[i].unit_normal, origin);
		frustum[i].type = PLANE_NON_AXIAL;
	}

	memcpy(&ren->current_view.frustum, &frustum, sizeof(frustum));
}
#endif

#if 0
void R_SetupEulerView(r32 pitch, r32 yaw, r32 roll, r32 view_orig_x, r32 view_orig_y, r32 view_orig_z) {
	Vec4 inv_trans_mat[4]; 
	Vec4 final[4];

	Vec4 rot_x[4], rot_y[4], rot_z[4];
	Vec4 tmp1[4];
	Vec4 tmp2[4];

	Vec4 tmp3[4];
	Vec4 tmp4[4];

	ViewSystem *vs = &ren.current_view;
	Matrix4x4SetIdentity(vs->view_matrix);
	Matrix4x4SetIdentity(vs->view_perspective_matrix);
	Matrix4x4SetIdentity(vs->perspective_screen_matrix);
	Vector3Init(vs->world_orientation.origin, view_orig_x, view_orig_y, view_orig_z);
	Vector3Copy(vs->debug_orientation.origin, vs->world_orientation.origin);
	Vec3 origin;
	Vector3Copy(origin, vs->world_orientation.origin);
	Vector3Negate(origin);
	Matrix3x3SetIdentity(ren.current_view.world_orientation.axis);


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
	//memcpy(vs->view_matrix, &tmp2, sizeof(tmp2));


	//Negate(rot_y[0].v.z);
	//Negate(rot_y[2].v.x);

	//Negate(rot_z[0].v.y);
	//Negate(rot_z[1].v.x);

	MatrixMultiply(&inv_trans_mat, &rot_x, &tmp3);
	MatrixMultiply(&tmp3, &rot_y, &tmp4);
	MatrixMultiply(&tmp4, &rot_z, &final);
	memcpy(vs->view_matrix, &final, sizeof(final));
}
#endif

void R_TransformWorldToView(Renderer *ren, MeshObject *md) {
	int num_verts = md->status.num_verts;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;
	for (int i = 0; i < num_verts; ++i) {
		r32 vert[4], tmp[4];
		Vector3Copy(vert, trans_verts[i]);
		vert[3] = 1.0f;

		Mat1x4Mul(tmp, vert, ren->current_view.view_matrix);  
		Vector3Copy(trans_verts[i], tmp);
	}
}

void R_TransformModelToWorld(Renderer *ren, MeshObject *md, VertexTransformState ts) {
	int num_verts = md->status.num_verts;
	Vec3 *local_verts = md->mesh->local_verts->vert_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	if (ts == VTS_LOCAL_TO_TRANSFORMED) {
		for (int i = 0; i < num_verts; ++i) {
			Vector3Add(local_verts[i], md->status.world_pos, trans_verts[i]);
		}
	} else if (ts == VTS_TRANSFORMED_ONLY) {
		for (int i = 0; i < num_verts; ++i) {
			Vector3Add(trans_verts[i], md->status.world_pos, trans_verts[i]);
		}
	}
}

FrustumClippingState R_CullPointAndRadius(Renderer *ren, Vec3 pt, r32 radius) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		Plane pl = ren->current_view.frustum[i];
		r32 dist = Vector3DotProduct(pt, pl.unit_normal) - pl.dist;

		if (dist < 0.0f) {
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
			//cull_state[i] = CULL_OUT;
			Sys_Print("Culling!!\n");
			return FCS_CULL_OUT;
			
		} 
	}

	//for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
	//	if (cull_state[i] == CULL_OUT) {
	//		Sys_Print("Point is culled!!\n");
	//		return CULL_OUT;
	//	}
	//}

	Sys_Print("NO culling!!\n");
	return FCS_CULL_IN;
	//Sys_Print("Point is on a plane of the frustum\n");
	//return CULL_OUT;
}

void R_TransformViewToClip(Renderer *ren, MeshObject *md) {
	int num_verts = md->status.num_verts;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;
	r32 (*m)[4] = ren->current_view.projection_matrix;
	r32 in[4];
	r32 out[4];

	for (int i = 0; i < num_verts; ++i) {
		// FIXME: macro
		in[0] = trans_verts[i][0];
		in[1] = trans_verts[i][1];
		in[2] = trans_verts[i][2];
		in[3] = 1.0f;

		Mat1x4Mul(out, in, m);  
		trans_verts[i][0] = out[0] / out[3];
		trans_verts[i][1] = out[1] / out[3];
		trans_verts[i][2] = out[2] / out[3];
		//z = trans_verts[i].v.z;
		//trans_verts[i].v.x = (trans_verts[i].v.x * dist) / z; 
		//trans_verts[i].v.y = (trans_verts[i].v.y * dist) / z; 
	}
}

void R_TransformClipToScreen(Renderer *ren, MeshObject *md) {
	int num_verts = md->status.num_verts;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	r32 screen_width_factor = (0.5f * ren->current_view.viewport_width) - 0.5f;
	r32 screen_height_factor = (0.5f * ren->current_view.viewport_height) - 0.5f;
	for (int i = 0; i < num_verts; ++i) {
		trans_verts[i][0] = screen_width_factor + (trans_verts[i][0] * screen_width_factor);
		trans_verts[i][1] = screen_height_factor - (trans_verts[i][1] * screen_height_factor);
	}
}

// Cohen-Sutherland clipping constants
#define INSIDE	 0	
#define LEFT	 1	
#define RIGHT	 2	
#define BOTTOM	 4	
#define TOP		 8  
static int R_GetLineClipCode(Renderer *ren, int x, int y) {
	int code = INSIDE;												// initialised as being inside of [[clip window]]

	if (x < 0) {													// to the left of clip window
		code |= LEFT;
	} else if (x >= ren->vid_sys.width) {			// to the right of clip window
		code |= RIGHT;
	}
	if (y < 0) {													// below the clip window
		code |= BOTTOM;
	} else if (y >= ren->vid_sys.height) {			// above the clip window
		code |= TOP;
	}

	return code;
}

// Cohen–Sutherland 
// FIXME: try Liang–Barsky or even Cyrus–Beck 
static void R_DrawLine(Renderer *ren, int x0, int y0, int x1, int y1, u32 color) {
	int outcode0 = R_GetLineClipCode(ren, x0, y0);
	int outcode1 = R_GetLineClipCode(ren, x1, y1);
	b32 accept = false;

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
				y = (r32)ren->vid_sys.height - 1.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & BOTTOM) { // point is below the clip rectangle
				y = 0.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & RIGHT) {  // point is to the right of clip rectangle
				x = (r32)ren->vid_sys.width - 1.0f;
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
				outcode0 = R_GetLineClipCode(ren, x0, y0);
			} else {
				x1 = (int)x;
				y1 = (int)y;
				outcode1 = R_GetLineClipCode(ren, x1, y1);
			}
		}
	}
	if (accept) {
		// Bresenham
		// FIXME: improve on this
		int dx			= abs(x1 - x0);
		int dy			= abs(y1 - y0);
		int pitch		= ren->vid_sys.pitch / ren->vid_sys.bpp;
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

		// x- or y-dominate line
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
		u32 *line = (u32*)ren->vid_sys.buffer;
		line = (line + (pitch * y0)) + x0;

		for (int i = 0; i <= num_pixels; ++i) {
			*line = color;
			numerator += add;
			if (numerator >= denominator) {
				numerator -= denominator;
				line += x2_inc;	// inc x in case of y-dominant line
				line += y2_inc; // inc y in case of x-dominant line
			}
			line += x1_inc;
			line += y1_inc;
		}
	}
}

static void R_DrawFlatBottomTriangle(Renderer *ren, r32 p0_x, r32 p0_y, r32 p1_x, r32 p1_y, r32 p2_x, r32 p2_y, Poly *poly) {
	if (p1_x < p2_x) {
		AnySwap(p1_x, p2_x, r32);
	}
	r32 dxy_left = (r32)(p2_x - p0_x) / (r32)(p2_y - p0_y);
	r32 dxy_right = (r32)(p1_x - p0_x) / (r32)(p1_y - p0_y);

	int cp0_y = (int)ceil(p0_y);
	int cp2_y = (int)(ceil(p2_y) - 1);
	r32 xs = p0_x;
	r32 xe = p0_x;
	xs = xs + ((cp0_y - p0_y) * dxy_left);
	xe = xe + ((cp0_y - p0_y) * dxy_right);
	for (int y = cp0_y; y <= cp2_y; ++y) {
		R_DrawLine(ren, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, poly->color); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

static void R_DrawFlatTopTriangle(Renderer *ren, r32 p0_x, r32 p0_y, r32 p1_x, r32 p1_y, r32 p2_x, r32 p2_y, Poly *poly) {
	if (p1_x < p0_x) {
		AnySwap(p1_x, p0_x, r32);
	}
	r32 dxy_left = (r32)(p2_x - p0_x) / (r32)(p2_y - p0_y);
	r32 dxy_right = (r32)(p2_x - p1_x) / (r32)(p2_y - p1_y);

	int cp0_y = (int)ceil(p0_y);
	int cp2_y = (int)(ceil(p2_y) - 1);
	r32 xs = p2_x;
	r32 xe = p2_x;
	xs = xs - ((cp0_y - p0_y) * dxy_left);
	xe = xe - ((cp0_y - p0_y) * dxy_right);
	for (int y = cp2_y; y >= cp0_y; --y) {
		R_DrawLine(ren, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, poly->color); 
		xs -= dxy_left;
		xe -= dxy_right;
	}
}

// FIXME: move out of the main rendering file into a different one
void R_DrawWireframeMesh(Renderer *ren, MeshObject *md) {
	int num_polys = md->status.num_polys;
	Poly *polys = md->mesh->polys->poly_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		R_DrawLine(ren,
				   (int)trans_verts[v0].v.x,
				   (int)trans_verts[v0].v.y,
				   (int)trans_verts[v1].v.x,
				   (int)trans_verts[v1].v.y,
				   polys[i].color);

		R_DrawLine(ren,
				   (int)trans_verts[v1].v.x,
				   (int)trans_verts[v1].v.y,
				   (int)trans_verts[v2].v.x,
				   (int)trans_verts[v2].v.y,
				   polys[i].color);

		R_DrawLine(ren,
				   (int)trans_verts[v2].v.x,
				   (int)trans_verts[v2].v.y,
				   (int)trans_verts[v0].v.x,
				   (int)trans_verts[v0].v.y,
				   polys[i].color);
	}
}

#if 1
void R_DrawSolidMesh(Renderer *ren, MeshObject *md) {
	int num_polys = md->status.num_polys;
	Poly *polys = md->mesh->polys->poly_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		r32 p0_x = trans_verts[v0].v.x;
		r32 p0_y = trans_verts[v0].v.y;

		r32 p1_x = trans_verts[v1].v.x;
		r32 p1_y = trans_verts[v1].v.y;

		r32 p2_x = trans_verts[v2].v.x;
		r32 p2_y = trans_verts[v2].v.y;

		// sort p0, p1, p2 in ascending y order
		if (p1_y < p0_y) {
			AnySwap(p1_x, p0_x, r32);
			AnySwap(p1_y, p0_y, r32);
		} 

		// now we know that p0 and p1 are in order 
		if (p2_y < p0_y) {
			AnySwap(p2_x, p0_x, r32);
			AnySwap(p2_y, p0_y, r32);
		} 

		if (p2_y < p1_y) {
			AnySwap(p2_x, p1_x, r32);
			AnySwap(p2_y, p1_y, r32);
		} 

		if (p0_y == p1_y) {
			// flat top
			R_DrawFlatTopTriangle(ren, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, &polys[i]);
		} else if (p1_y == p2_y) {
			// flat bottom
			R_DrawFlatBottomTriangle(ren, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, &polys[i]);
		} else {
			// m = (y - y0) / (x - x0)
			// m(x - x0) = y - y0
			// mx - mx0 = y - y0
			// x - x0 = (y - y0) / m
			// x = (y - y0) / m + x0
			// x derived from the point-slope form of the line
			r32 m = (p2_y - p0_y) / (p2_x - p0_x);
			r32 x = (p1_y - p0_y) / m + p0_x;
			//x = (r32)((int)(x + 0.5f));
			R_DrawFlatBottomTriangle(ren, p0_x, p0_y, x, p1_y, p1_x, p1_y, &polys[i]);
			R_DrawFlatTopTriangle(ren, p1_x, p1_y, x, p1_y, p2_x, p2_y, &polys[i]);
		}
	}
}
#endif


void R_RotatePoints(r32 rot_mat[3][3], Vec3 *points, int num_verts) {
	for (int i = 0; i < num_verts; ++i) {
		r32 x = Vector3DotProduct(rot_mat[0], points[i]);
		r32 y = Vector3DotProduct(rot_mat[1], points[i]);
		r32 z = Vector3DotProduct(rot_mat[2], points[i]);

		points[i][0] = x;
		points[i][1] = y;
		points[i][2] = z;
	}
}

void R_CullBackFaces(Renderer *ren, MeshObject *md) {
	Poly *polys = md->mesh->polys->poly_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;
	Vec3 p = {};
	int num_polys = md->status.num_polys;

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE) || !(polys[i].state & POLY_STATE_ACTIVE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		Vec3 u = Vector3Build(trans_verts[v0], trans_verts[v1]);
		Vec3 v = Vector3Build(trans_verts[v0], trans_verts[v2]);
		Vec3 n = Vector3CrossProduct(u, v);
		Vec3 view = Vector3Build(trans_verts[v0], p);

		r32 dot = Vector3DotProduct(view, n);

		if (dot <= 0.0f) {
			polys[i].state |= POLY_STATE_BACKFACE;
		}
	}
}

void R_RotateForViewer(Renderer *ren) {
	r32		view_matrix[16];
	Vec3	origin;

	Vector3Copy(origin, ren->current_view.world_orientation.origin);

	// compute the uvn vectors
	Vec3 n = ren->current_view.world_orientation.dir;
	// placeholder for v
	Vec3 v = {0.0f, 1.0f, 0.0f};	
	Vec3 u = Vector3CrossProduct(v, n);
	// recompute v
	v = Vector3CrossProduct(n, u);

	u = Vector3Normalize(u);
	v = Vector3Normalize(v);
	n = Vector3Normalize(n);

	ren->current_view.world_orientation.axis[0] = u;
	ren->current_view.world_orientation.axis[1] = v;
	ren->current_view.world_orientation.axis[2] = n;

	view_matrix[0] = u[0];
	view_matrix[4] = u[1];
	view_matrix[8] = u[2];
	view_matrix[12] = -(Vector3DotProduct(u, origin));

	view_matrix[1] = v[0];
	view_matrix[5] = v[1];
	view_matrix[9] = v[2];
	view_matrix[13] = -(Vector3DotProduct(v, origin));

	view_matrix[2] = n[0];
	view_matrix[6] = n[1];
	view_matrix[10] = n[2];
	view_matrix[14] = -(Vector3DotProduct(n, origin));

	view_matrix[3] = 0.0f;
	view_matrix[7] = 0.0f;
	view_matrix[11] = 0.0f;
	view_matrix[15] = 1.0f;

	memcpy(ren->current_view.view_matrix, view_matrix, sizeof(view_matrix));
}

void R_RenderView(Renderer *ren) {
	static b32 first_draw = false;
	if (!first_draw) {
		first_draw = true;

		Vector3Init(ren->current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);
		Vector3Init(ren->current_view.target, 0.0f, 0.0f, 1.0f);

		ren->current_view.world_orientation.dir = ren->current_view.target;
		ren->current_view.aspect_ratio = (r32)ren->vid_sys.width / (r32)ren->vid_sys.height;

		ren->current_view.viewplane_width = 2.0f;	// normalized viewplane
		ren->current_view.viewplane_height = 2.0f;

		ren->current_view.fov_y = 75.0f;

		// FIXME: compute dynamically
		ren->current_view.z_near = 50.0f;
		ren->current_view.z_far = 500.0f;

		ren->current_view.viewport_width = (r32)ren->vid_sys.width;		
		ren->current_view.viewport_height = (r32)ren->vid_sys.height;	
	}

	//if ( parms->viewportWidth <= 0 || parms->viewportHeight <= 0 ) {
	//	return;
	//}

	//tr.viewCount++;

	//tr.viewParms = *parms;
	//tr.viewParms.frameSceneNum = tr.frameSceneNum;
	//tr.viewParms.frameCount = tr.frameCount;

	//firstDrawSurf = tr.refdef.numDrawSurfs;

	//tr.viewCount++;

	// set viewParms.world

	R_RotateForViewer(ren);
	//R_SetupFrustum(ren);					
	R_SetupProjection(ren);


	//R_GenerateDrawSurfs();

	//R_SortDrawSurfs( tr.refdef.drawSurfs + firstDrawSurf, tr.refdef.numDrawSurfs - firstDrawSurf );

	//// draw main system development information (surface outlines, etc)
	//R_DebugGraphics();
}
