#include "renderer.h"
#include "win_r.h"
#include "plg_loader.h"

void R_SetupProjection(ViewSystem *vs) {
	r32 aspect_ratio = vs->aspect_ratio;
	r32 fov_y = vs->fov_y;

	// NOTE: currently this handles only view planes 2x2 dimension
	// FIXME: handle variable sized view planes
	r32 d = (vs->viewplane_width / 2.0f) / tan(DEG2RAD(fov_y / 2.0f));

	// D3D style [0, 1] z-buffer mapping
	r32 a = vs->z_far / (vs->z_far - vs->z_near);
	r32 b = -vs->z_near * a;

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

	vs->view_dist = d;
	memcpy(vs->projection_matrix, projection_matrix, sizeof(projection_matrix));
}

// Gribb & Hartmann method
void R_SetupFrustum(ViewSystem *vs) {
	Plane frustum[NUM_FRUSTUM_PLANES];
	r32	combo_matrix[4][4];
	Mat4x4Mul(combo_matrix, vs->view_matrix, vs->projection_matrix);

	// exctract the planes in world space
	frustum[FRUSTUM_PLANE_LEFT].unit_normal[0] = combo_matrix[0][0] + combo_matrix[0][3];
	frustum[FRUSTUM_PLANE_LEFT].unit_normal[1] = combo_matrix[1][0] + combo_matrix[1][3];
	frustum[FRUSTUM_PLANE_LEFT].unit_normal[2] = combo_matrix[2][0] + combo_matrix[2][3];
	frustum[FRUSTUM_PLANE_LEFT].dist = combo_matrix[3][0] + combo_matrix[3][3];

	frustum[FRUSTUM_PLANE_RIGHT].unit_normal[0] = -combo_matrix[0][0] + combo_matrix[0][3];
	frustum[FRUSTUM_PLANE_RIGHT].unit_normal[1] = -combo_matrix[1][0] + combo_matrix[1][3];
	frustum[FRUSTUM_PLANE_RIGHT].unit_normal[2] = -combo_matrix[2][0] + combo_matrix[2][3];
	frustum[FRUSTUM_PLANE_RIGHT].dist = -combo_matrix[3][0] - combo_matrix[3][3];

	frustum[FRUSTUM_PLANE_TOP].unit_normal[0] = -combo_matrix[0][1] + combo_matrix[0][3];
	frustum[FRUSTUM_PLANE_TOP].unit_normal[1] = -combo_matrix[1][1] + combo_matrix[1][3];
	frustum[FRUSTUM_PLANE_TOP].unit_normal[2] = -combo_matrix[2][1] + combo_matrix[2][3];
	frustum[FRUSTUM_PLANE_TOP].dist = -combo_matrix[3][1] - combo_matrix[3][3];

	frustum[FRUSTUM_PLANE_BOTTOM].unit_normal[0] = combo_matrix[0][1] + combo_matrix[0][3];
	frustum[FRUSTUM_PLANE_BOTTOM].unit_normal[1] = combo_matrix[1][1] + combo_matrix[1][3];
	frustum[FRUSTUM_PLANE_BOTTOM].unit_normal[2] = combo_matrix[2][1] + combo_matrix[2][3];
	frustum[FRUSTUM_PLANE_BOTTOM].dist = combo_matrix[3][1] - combo_matrix[3][3];

#if 0
	frustum[FRUSTUM_PLANE_NEAR].unit_normal[0] = combo_matrix[0][2] + combo_matrix[0][3];
	frustum[FRUSTUM_PLANE_NEAR].unit_normal[1] = combo_matrix[1][2] + combo_matrix[1][3];
	frustum[FRUSTUM_PLANE_NEAR].unit_normal[2] = combo_matrix[2][2] + combo_matrix[2][3];
	frustum[FRUSTUM_PLANE_NEAR].dist = combo_matrix[3][2] - combo_matrix[3][3];
	int y = 42;
#endif
	// FIXME: add near and far planes

	// normalize
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		frustum[i].unit_normal = Vec3Norm(frustum[i].unit_normal);
		frustum[i].dist = Vec3Dot(frustum[i].unit_normal, vs->world_orientation.origin);
	}

	int x = 42;
	memcpy(&vs->frustum, &frustum, sizeof(frustum));
}

void R_TransformWorldToView(ViewSystem *vs, Vec3 *poly_verts, int num_verts) {
	for (int i = 0; i < num_verts; ++i) {
		r32 vert[4], tmp[4];
		Vec3Copy(vert, poly_verts[i]);
		vert[3] = 1.0f;

		Mat1x4Mul(tmp, vert, vs->view_matrix);  
		Vec3Copy(poly_verts[i], tmp);
	}
}

void R_TransformModelToWorld(Vec3 *local_poly_verts, Vec3 *trans_poly_verts, int num_verts, Vec3 world_pos) {
	for (int i = 0; i < num_verts; ++i) {
		Vector3Add(local_poly_verts[i], world_pos, trans_poly_verts[i]);
	}
}

ClipFlags R_CullPointAndRadius(ViewSystem *vs, Vec3 pt, r32 radius) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		Plane pl = vs->frustum[i];
		r32 dist = Vec3Dot(pt, pl.unit_normal) - pl.dist;

		if (dist < 0.0f) {
			//Sys_Print("Culling!!\n");
			return CULL_OUT;
		} 
	}

	//Sys_Print("NO culling!!\n");
	return CULL_IN;
}

void R_TransformViewToClip(ViewSystem *vs, Vec3 *poly_verts, int num_verts) {
	r32 (*m)[4] = vs->projection_matrix;
	r32 in[4];
	r32 out[4];

	for (int i = 0; i < num_verts; ++i) {
		in[0] = poly_verts[i][0];
		in[1] = poly_verts[i][1];
		in[2] = poly_verts[i][2];
		in[3] = 1.0f;

		Mat1x4Mul(out, in, m);  
		poly_verts[i][0] = out[0] / out[3];
		poly_verts[i][1] = out[1] / out[3];
		poly_verts[i][2] = out[2] / out[3];
	}
}

void R_TransformClipToScreen(ViewSystem *vs, Vec3 *poly_verts, int num_verts) {
	r32 screen_width_factor = (0.5f * vs->viewport_width) - 0.5f;
	r32 screen_height_factor = (0.5f * vs->viewport_height) - 0.5f;
	for (int i = 0; i < num_verts; ++i) {
		poly_verts[i][0] = screen_width_factor + (poly_verts[i][0] * screen_width_factor);
		poly_verts[i][1] = screen_height_factor + (poly_verts[i][1] * screen_height_factor);
	}
}

void R_RotatePoints(r32 rot_mat[3][3], Vec3 *points, int num_verts) {
	for (int i = 0; i < num_verts; ++i) {
		r32 x = Vec3Dot(rot_mat[0], points[i]);
		r32 y = Vec3Dot(rot_mat[1], points[i]);
		r32 z = Vec3Dot(rot_mat[2], points[i]);

		points[i][0] = x;
		points[i][1] = y;
		points[i][2] = z;
	}
}

void R_CullBackFaces(ViewSystem *vs, Poly *polys, const Vec3 *poly_verts, int num_polys) {
	Vec3 p = {};

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE) || !(polys[i].state & POLY_STATE_ACTIVE)) {
			continue;
		}

		Vec3 v0 = polys[i].vertex_array[0];
		Vec3 v1 = polys[i].vertex_array[1];
		Vec3 v2 = polys[i].vertex_array[2];
		Vec3 u = MakeVec3(v0, v1);
		Vec3 v = MakeVec3(v0, v2);
		Vec3 n = Vec3Cross(u, v);
		Vec3 view = MakeVec3(v0, p);

		r32 dot = Vec3Dot(view, n);
		if (dot <= 0.0f) {
			polys[i].state |= POLY_STATE_BACKFACE;
		}
	}
}

static void R_RotateForViewer(ViewSystem *vs) {
	r32		view_matrix[16];
	Vec3	origin;

	Vec3Copy(origin, vs->world_orientation.origin);

	// compute the uvn vectors
	Vec3 n = vs->world_orientation.dir;
	// placeholder for v
	Vec3 v = {0.0f, 1.0f, 0.0f};	
	Vec3 u = Vec3Cross(v, n);
	// recompute v
	v = Vec3Cross(n, u);

	u = Vec3Norm(u);
	v = Vec3Norm(v);
	n = Vec3Norm(n);

	vs->world_orientation.axis[0] = u;
	vs->world_orientation.axis[1] = v;
	vs->world_orientation.axis[2] = n;

	view_matrix[0] = u[0];
	view_matrix[4] = u[1];
	view_matrix[8] = u[2];
	view_matrix[12] = -(Vec3Dot(u, origin));

	view_matrix[1] = v[0];
	view_matrix[5] = v[1];
	view_matrix[9] = v[2];
	view_matrix[13] = -(Vec3Dot(v, origin));

	view_matrix[2] = n[0];
	view_matrix[6] = n[1];
	view_matrix[10] = n[2];
	view_matrix[14] = -(Vec3Dot(n, origin));

	view_matrix[3] = 0.0f;
	view_matrix[7] = 0.0f;
	view_matrix[11] = 0.0f;
	view_matrix[15] = 1.0f;

	memcpy(vs->view_matrix, view_matrix, sizeof(view_matrix));
}

void R_RenderView(ViewSystem *vs) {
	R_RotateForViewer(vs);
	R_SetupProjection(vs);
	R_SetupFrustum(vs);					
}

void R_AddPolys(RendererBackend *rb, const Vec3 *verts, Poly *poly_array, int num_polys) {
	Poly *poly;
	int num_verts = 3;	// triangle
	Assert(rb->num_polys + num_polys <= MAX_NUM_POLYS);

	for (int i = 0; i < num_polys; ++i) {
		poly = &rb->polys[rb->num_polys];
		poly->num_verts = num_verts;
		poly->state = poly_array[i].state;
		poly->color = poly_array[i].color;
		poly->vertex_array = &rb->poly_verts[rb->num_verts];

		//memcpy(poly->vertex_array, &verts[*poly_array[i].vert_indices], num_verts * sizeof(*verts));
		for (int j = 0; j < num_verts; ++j) {
			poly->vertex_array[j] = verts[poly_array[i].vert_indices[j]];
		}

		++rb->num_polys; 
		rb->num_verts += num_verts;
		//verts += num_verts;
	}
}
