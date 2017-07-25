#include "renderer.h"
#include "win_r.h"
#include "plg_loader.h"

void RF_SetupProjection(ViewSystem *vs) {
	r32 aspect_ratio = vs->aspect_ratio;
	r32 fov_y = vs->fov_y;
	r32 z_near = vs->z_near, z_far = vs->z_far;
	r32 w = (r32)vs->viewplane_width, h = (r32)vs->viewplane_height;
	r32 left = -(w / 2.0f), right = (w / 2.0f); 
	r32 bottom = -(h / 2.0f), top = (h / 2.0f); 

	// NOTE: currently this handles only view planes 2x2 dimension
	// FIXME: handle variable sized view planes
	r32 d = (w / 2.0f) / tan(DEG2RAD(fov_y / 2.0f));

	// D3D style [0, 1] z-buffer mapping
	r32 a = z_far / (z_far - z_near);
	r32 b = -z_near * a;

	r32 m00 = 1.0f / aspect_ratio;
	r32 projection_matrix[16];

	projection_matrix[0] = (2.0f * m00 * d) / (right - left);
	projection_matrix[4] = 0.0f;
	projection_matrix[8] = 0.0f;
	projection_matrix[12] = 0.0f;

	projection_matrix[1] = 0.0f;
	projection_matrix[5] = (2.0f * d) / (top - bottom);
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
void RF_SetupFrustum(ViewSystem *vs) {
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

#if 1
	frustum[FRUSTUM_PLANE_NEAR].unit_normal[0] = combo_matrix[0][2];
	frustum[FRUSTUM_PLANE_NEAR].unit_normal[1] = combo_matrix[1][2];
	frustum[FRUSTUM_PLANE_NEAR].unit_normal[2] = combo_matrix[2][2];
	frustum[FRUSTUM_PLANE_NEAR].dist = combo_matrix[3][2];
	int y = 42;
#endif

	// normalize
	for (int i = 0; i < NUM_FRUSTUM_PLANES - 1; ++i) {
		frustum[i].unit_normal = Vec3Norm(frustum[i].unit_normal);
		frustum[i].dist = Dot3(frustum[i].unit_normal, vs->world_orientation.origin);
	}
	for (int i = NUM_FRUSTUM_PLANES - 1; i < NUM_FRUSTUM_PLANES; ++i) {
		frustum[i].unit_normal = Vec3Norm(frustum[i].unit_normal);
		frustum[i].dist = Dot3(frustum[i].unit_normal, vs->world_orientation.origin + (vs->world_orientation.dir * vs->z_near));
	}

	memcpy(&vs->frustum, &frustum, sizeof(frustum));
}

void RF_TransformWorldToView(ViewSystem *vs, PolyVert *poly_verts, int num_verts) {
	for (int i = 0; i < num_verts; ++i) {
		Vec4 p, tmp;
		Vec3Copy(p, poly_verts[i].xyz);
		p[3] = 1.0f;

		Mat1x4Mul(&p, &p, vs->view_matrix);  
		Vec3Copy(poly_verts[i].xyz, p);
	}
}

void RF_TransformModelToWorld(PolyVert *local_poly_verts, PolyVert *trans_poly_verts, int num_verts, Vec3 world_pos, r32 world_scale) {
	for (int i = 0; i < num_verts; ++i) {
		trans_poly_verts[i].xyz = (local_poly_verts[i].xyz * world_scale) + world_pos;
	}
}

ClipFlags RF_CullPointAndRadius(ViewSystem *vs, Vec3 pt, r32 radius) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		Plane pl = vs->frustum[i];
		r32 dist = Dot3(pt, pl.unit_normal) - pl.dist;

		if (dist < 0.0f) {
			//Sys_Print("Culling!!\n");
			return CULL_OUT;
		} 
	}

	//Sys_Print("NO culling!!\n");
	return CULL_IN;
}

void RF_TransformViewToClip(ViewSystem *vs, PolyVert *poly_verts, int num_verts) {
	r32 (*m)[4] = vs->projection_matrix;

	for (int i = 0; i < num_verts; ++i) {
		r32 in[4];
		r32 out[4];

		in[0] = poly_verts[i].xyz[0];
		in[1] = poly_verts[i].xyz[1];
		in[2] = poly_verts[i].xyz[2];
		in[3] = 1.0f;

		Mat1x4Mul(out, in, m);  
		poly_verts[i].xyz[0] = (out[0] / out[3]);
		poly_verts[i].xyz[1] = (out[1] / out[3]);
		poly_verts[i].xyz[2] = (out[2] / out[3]);
	}
}

void RF_TransformClipToScreen(ViewSystem *vs, PolyVert *poly_verts, int num_verts) {
	r32 in[3];
	r32 screen_width_factor = (0.5f * (vs->viewport_width - 1.0f));
	r32 screen_height_factor = (0.5f * (vs->viewport_height - 1.0f));
	for (int i = 0; i < num_verts; ++i) {
		poly_verts[i].xyz[0] = (screen_width_factor + (poly_verts[i].xyz[0] * screen_width_factor));
		poly_verts[i].xyz[1] = (screen_height_factor + (poly_verts[i].xyz[1] * screen_height_factor));
	}
}

void RotatePoints(r32 rot_mat[3][3], PolyVert *poly_verts, int num_verts) {
	for (int i = 0; i < num_verts; ++i) {
		r32 x = Dot3(rot_mat[0], poly_verts[i].xyz);
		r32 y = Dot3(rot_mat[1], poly_verts[i].xyz);
		r32 z = Dot3(rot_mat[2], poly_verts[i].xyz);

		poly_verts[i].xyz[0] = x;
		poly_verts[i].xyz[1] = y;
		poly_verts[i].xyz[2] = z;
	}
}

// culling in view space
void RF_CullBackFaces(ViewSystem *vs, Poly *polys, int num_polys) {
	Vec3 p = {0.0f, 0.0f, 0.0f};

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE) || !(polys[i].state & POLY_STATE_ACTIVE)) {
			continue;
		}

		PolyVert v0 = polys[i].vertex_array[0];
		PolyVert v1 = polys[i].vertex_array[1];
		PolyVert v2 = polys[i].vertex_array[2];
		Vec3 u = MakeVec3(v0.xyz, v1.xyz);
		Vec3 v = MakeVec3(v0.xyz, v2.xyz);
		Vec3 n = -Cross3(u, v);		// NOTE: negated because of left-handed system and because we are using ccw winding order
		Vec3 view = MakeVec3(v0.xyz, p);

		r32 dot = Dot3(view, n);
		if (dot < 0.0f) {
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
	Vec3 u = Cross3(v, n);
	// recompute v
	v = Cross3(n, u);

	u = Vec3Norm(u);
	v = Vec3Norm(v);
	n = Vec3Norm(n);

	vs->world_orientation.axis[0] = u;
	vs->world_orientation.axis[1] = v;
	vs->world_orientation.axis[2] = n;

	view_matrix[0] = u[0];
	view_matrix[4] = u[1];
	view_matrix[8] = u[2];
	view_matrix[12] = -(Dot3(u, origin));

	view_matrix[1] = v[0];
	view_matrix[5] = v[1];
	view_matrix[9] = v[2];
	view_matrix[13] = -(Dot3(v, origin));

	view_matrix[2] = n[0];
	view_matrix[6] = n[1];
	view_matrix[10] = n[2];
	view_matrix[14] = -(Dot3(n, origin));

	view_matrix[3] = 0.0f;
	view_matrix[7] = 0.0f;
	view_matrix[11] = 0.0f;
	view_matrix[15] = 1.0f;

	memcpy(vs->view_matrix, view_matrix, sizeof(view_matrix));
}

void RF_RenderView(ViewSystem *vs) {
	R_RotateForViewer(vs);
	RF_SetupProjection(vs);
	RF_SetupFrustum(vs);					
}

void RF_AddPolys(RendererBackend *rb, const PolyVert *verts, Poly *poly_array, int num_polys) {
	int num_verts = 3;	// triangle
	Assert(rb->num_polys + num_polys <= MAX_NUM_POLYS);

	for (int i = 0; i < num_polys; ++i) {
		Poly *poly = &rb->polys[rb->num_polys];
		poly->num_verts = num_verts;
		poly->state = poly_array[i].state;
		poly->color = poly_array[i].color;
		poly->vertex_array = &rb->poly_verts[rb->num_poly_verts];

		for (int j = 0; j < num_verts; ++j) {
			poly->vertex_array[j] = verts[poly_array[i].vert_indices[j]];
		}

		++rb->num_polys; 
		rb->num_poly_verts += num_verts;
	}
}

void RF_CalculateVertexNormals(Poly *polys, int num_polys, PolyVert *poly_verts, int num_poly_verts) {
	for (int i = 0; i < num_poly_verts; ++i) {
		poly_verts[i].normal[0] = 0.0f;
		poly_verts[i].normal[1] = 0.0f;
		poly_verts[i].normal[2] = 0.0f;
	}
	// FIXME: function to compute poly normals
	for (int i = 0; i < num_polys; ++i) {
		if (polys[i].state & POLY_STATE_BACKFACE) {
			continue;
		}
		PolyVert v0 = polys[i].vertex_array[0];
		PolyVert v1 = polys[i].vertex_array[1];
		PolyVert v2 = polys[i].vertex_array[2];
		Vec3 u = MakeVec3(v0.xyz, v1.xyz);
		Vec3 v = MakeVec3(v0.xyz, v2.xyz);
		Vec3 n = -Cross3(u, v);
		v0.normal = v0.normal + n;
		v1.normal = v1.normal + n;
		v2.normal = v2.normal + n;
		polys[i].vertex_array[0].normal = v0.normal;
		polys[i].vertex_array[1].normal = v1.normal;
		polys[i].vertex_array[2].normal = v2.normal;
	}
	for (int i = 0; i < num_poly_verts; ++i) {
		poly_verts[i].normal = Vec3Norm(poly_verts[i].normal);
	}
}
