#include "renderer.h"
#include "win_r.h"

void RF_SetupProjection(ViewSystem *vs) {
	r32 projection_matrix[16];
	r32 aspect_ratio = vs->aspect_ratio;
	r32 fov_y = vs->fov_y;
	r32 z_near = vs->z_near, z_far = vs->z_far;
	r32 w = (r32)vs->viewplane_width, h = (r32)vs->viewplane_height;
	r32 left = -(w / 2.0f), right = (w / 2.0f); 
	r32 bottom = -(h / 2.0f), top = (h / 2.0f); 

	// FIXME: handle variable sized view planes
	r32 d = (w / 2.0f) / (r32)tan(DEG2RAD(fov_y / 2.0f));

	// D3D style [0, 1] z-buffer mapping
	r32 a = z_far / (z_far - z_near);
	r32 b = -z_near * a;

	r32 m00 = 1.0f / aspect_ratio;

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

void RF_ExtractWorldPlanes(ViewSystem *vs) {
	Plane wps[NUM_FRUSTUM_PLANES];
	r32	combo_matrix[4][4];
	Mat4x4Mul(combo_matrix, vs->view_matrix, vs->projection_matrix);

	// exctract the planes in world space
	wps[FRUSTUM_PLANE_LEFT].unit_normal[0] = combo_matrix[0][0] + combo_matrix[0][3];
	wps[FRUSTUM_PLANE_LEFT].unit_normal[1] = combo_matrix[1][0] + combo_matrix[1][3];
	wps[FRUSTUM_PLANE_LEFT].unit_normal[2] = combo_matrix[2][0] + combo_matrix[2][3];
	wps[FRUSTUM_PLANE_LEFT].dist = combo_matrix[3][0] + combo_matrix[3][3];

	wps[FRUSTUM_PLANE_RIGHT].unit_normal[0] = -combo_matrix[0][0] + combo_matrix[0][3];
	wps[FRUSTUM_PLANE_RIGHT].unit_normal[1] = -combo_matrix[1][0] + combo_matrix[1][3];
	wps[FRUSTUM_PLANE_RIGHT].unit_normal[2] = -combo_matrix[2][0] + combo_matrix[2][3];
	wps[FRUSTUM_PLANE_RIGHT].dist = -combo_matrix[3][0] - combo_matrix[3][3];

	wps[FRUSTUM_PLANE_TOP].unit_normal[0] = -combo_matrix[0][1] + combo_matrix[0][3];
	wps[FRUSTUM_PLANE_TOP].unit_normal[1] = -combo_matrix[1][1] + combo_matrix[1][3];
	wps[FRUSTUM_PLANE_TOP].unit_normal[2] = -combo_matrix[2][1] + combo_matrix[2][3];
	wps[FRUSTUM_PLANE_TOP].dist = -combo_matrix[3][1] - combo_matrix[3][3];

	wps[FRUSTUM_PLANE_BOTTOM].unit_normal[0] = combo_matrix[0][1] + combo_matrix[0][3];
	wps[FRUSTUM_PLANE_BOTTOM].unit_normal[1] = combo_matrix[1][1] + combo_matrix[1][3];
	wps[FRUSTUM_PLANE_BOTTOM].unit_normal[2] = combo_matrix[2][1] + combo_matrix[2][3];
	wps[FRUSTUM_PLANE_BOTTOM].dist = combo_matrix[3][1] - combo_matrix[3][3];

	wps[FRUSTUM_PLANE_NEAR].unit_normal[0] = combo_matrix[0][2];
	wps[FRUSTUM_PLANE_NEAR].unit_normal[1] = combo_matrix[1][2];
	wps[FRUSTUM_PLANE_NEAR].unit_normal[2] = combo_matrix[2][2];
	wps[FRUSTUM_PLANE_NEAR].dist = combo_matrix[3][2];

	// normalize
	for (int i = 0; i < NUM_FRUSTUM_PLANES - 1; ++i) {
		wps[i].unit_normal = Vec3Norm(wps[i].unit_normal);
		wps[i].dist = Dot3(wps[i].unit_normal, vs->world_orientation.origin);
	}
	wps[FRUSTUM_PLANE_NEAR].unit_normal = Vec3Norm(wps[FRUSTUM_PLANE_NEAR].unit_normal);
	wps[FRUSTUM_PLANE_NEAR].dist = Dot3(wps[FRUSTUM_PLANE_NEAR].unit_normal, vs->world_orientation.origin + (vs->world_orientation.dir * vs->z_near));

	memcpy(&vs->frustum, &wps, sizeof(wps));
}

void RF_ExtractViewPlanes(ViewSystem *vs) {
	Plane vps[NUM_FRUSTUM_PLANES];
	r32	pm[4][4];
	memcpy(&pm, vs->projection_matrix, sizeof(pm));

	// exctract the planes in view space
	vps[FRUSTUM_PLANE_LEFT].unit_normal[0] = pm[0][0] + pm[0][3];
	vps[FRUSTUM_PLANE_LEFT].unit_normal[1] = pm[1][0] + pm[1][3];
	vps[FRUSTUM_PLANE_LEFT].unit_normal[2] = pm[2][0] + pm[2][3];
	vps[FRUSTUM_PLANE_LEFT].dist = pm[3][0] + pm[3][3];

	vps[FRUSTUM_PLANE_RIGHT].unit_normal[0] = -pm[0][0] + pm[0][3];
	vps[FRUSTUM_PLANE_RIGHT].unit_normal[1] = -pm[1][0] + pm[1][3];
	vps[FRUSTUM_PLANE_RIGHT].unit_normal[2] = -pm[2][0] + pm[2][3];
	vps[FRUSTUM_PLANE_RIGHT].dist = -pm[3][0] - pm[3][3];

	vps[FRUSTUM_PLANE_TOP].unit_normal[0] = -pm[0][1] + pm[0][3];
	vps[FRUSTUM_PLANE_TOP].unit_normal[1] = -pm[1][1] + pm[1][3];
	vps[FRUSTUM_PLANE_TOP].unit_normal[2] = -pm[2][1] + pm[2][3];
	vps[FRUSTUM_PLANE_TOP].dist = -pm[3][1] - pm[3][3];

	vps[FRUSTUM_PLANE_BOTTOM].unit_normal[0] = pm[0][1] + pm[0][3];
	vps[FRUSTUM_PLANE_BOTTOM].unit_normal[1] = pm[1][1] + pm[1][3];
	vps[FRUSTUM_PLANE_BOTTOM].unit_normal[2] = pm[2][1] + pm[2][3];
	vps[FRUSTUM_PLANE_BOTTOM].dist = pm[3][1] - pm[3][3];

	vps[FRUSTUM_PLANE_NEAR].unit_normal[0] = pm[0][2];
	vps[FRUSTUM_PLANE_NEAR].unit_normal[1] = pm[1][2];
	vps[FRUSTUM_PLANE_NEAR].unit_normal[2] = pm[2][2];
	vps[FRUSTUM_PLANE_NEAR].dist = pm[3][2];

	// normalize
	for (int i = 0; i < NUM_FRUSTUM_PLANES-1; ++i) {
		vps[i].unit_normal = Vec3Norm(vps[i].unit_normal);
		vps[i].dist = Dot3(vps[i].unit_normal, MV3(0.0f, 0.0f, 0.0f));
	}
	vps[FRUSTUM_PLANE_NEAR].unit_normal = Vec3Norm(vps[FRUSTUM_PLANE_NEAR].unit_normal);
	vps[FRUSTUM_PLANE_NEAR].dist = Dot3(vps[FRUSTUM_PLANE_NEAR].unit_normal, MV3(0.0f, 0.0f, vs->z_near));

   memcpy(&vs->view_planes, vps, sizeof(vs->view_planes));
}


void RF_TransformWorldToView(ViewSystem *vs, PolyVert *poly_verts, int num_verts) {
	for (int i = 0; i < num_verts; ++i) {
		Vec4 p;
		Vec3Copy(p, poly_verts[i].xyz);
		p[3] = 1.0f;

		Mat1x4Mul(&p, &p, vs->view_matrix);  
		Vec3Copy(poly_verts[i].xyz, p);
	}
}

void RF_TransformModelToWorld(const PolyVert *model_poly_verts, PolyVert *trans_poly_verts, int num_verts, Vec3 world_pos, r32 world_scale) {
	for (int i = 0; i < num_verts; ++i) {
      trans_poly_verts[i] = model_poly_verts[i];
		trans_poly_verts[i].xyz = trans_poly_verts[i].xyz*world_scale + world_pos;
	}
}

// culling in worldspace
#if 1
int RF_CullPointAndRadius(ViewSystem *vs, Vec3 pt, r32 radius) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		Plane pl = vs->frustum[i];
		r32 dist = Dot3(pt, pl.unit_normal) - pl.dist;

		if ((dist-radius) < 0.0f) {
			//Sys_Print("Culling!!\n");
			return CULL_OUT;
		} 
	}

	//Sys_Print("NO culling!!\n");
	return CULL_IN;
}
#endif

// culling in viewspace
int RF_CullPoly(ViewSystem *vs, Poly *poly) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
      Plane pl = vs->view_planes[i];
      Vec3 p0 = poly->vertex_array[0].xyz;
      Vec3 p1 = poly->vertex_array[1].xyz;
      Vec3 p2 = poly->vertex_array[2].xyz;
      r32 dist0 = Dot3(p0, pl.unit_normal) - pl.dist;
      r32 dist1 = Dot3(p1, pl.unit_normal) - pl.dist;
      r32 dist2 = Dot3(p2, pl.unit_normal) - pl.dist;

      if (dist0 < 0.0f && dist1 < 0.0f && dist2 < 0.0f) {
         Sys_Print("Culling polygon!!\n");
         return CULL_OUT;
      } 
	}

   // poly completely inside the frustum
   return CULL_IN;
}


void RF_TransformViewToProj(ViewSystem *vs, PolyVert *poly_verts, int num_verts) {
   for (int i = 0; i < num_verts; ++i) {
      r32 in[4];
      r32 out[4];

      in[0] = poly_verts[i].xyz[0];
      in[1] = poly_verts[i].xyz[1];
      in[2] = poly_verts[i].xyz[2];
      in[3] = 1.0f;

      Mat1x4Mul(out, in, vs->projection_matrix);  
      if (out[3] > 0.0f) {
         poly_verts[i].xyz[0] = out[0];
         poly_verts[i].xyz[1] = out[1];
         poly_verts[i].xyz[2] = out[2];
         poly_verts[i].w = out[3];
      }
   }
}

void RF_TransformProjToClip(PolyVert *poly_verts, int num_verts) {
   for (int i = 0; i < num_verts; ++i) {
      r32 w = poly_verts[i].w;
      Assert(w > 0.0f);
      poly_verts[i].xyz[0] /= w;
      poly_verts[i].xyz[1] /= w;
      poly_verts[i].xyz[2] /= w;
   }
}

void RF_TransformClipToScreen(ViewSystem *vs, PolyVert *poly_verts, int num_verts) {
	//s32 screen_width_factor = vs->viewport_width >> 1;
	//s32 screen_height_factor = vs->viewport_height >> 1;
   r32 kx = (r32)((vs->viewport_width-1) >> 1);
   r32 ky = (r32)((vs->viewport_height-1) >> 1);
	for (int i = 0; i < num_verts; ++i) {
      r32 px = poly_verts[i].xyz[0] + 1.0f;
      r32 py = poly_verts[i].xyz[1] + 1.0f;
      poly_verts[i].xyz[0] = px*kx - kx;
		poly_verts[i].xyz[1] = py*ky - ky;

      //poly_verts[i].xyz[0] = (poly_verts[i].xyz[0] + 1) * screen_width_factor;
		//poly_verts[i].xyz[1] = (poly_verts[i].xyz[1] + 1) * screen_height_factor;
	}
}

void RotateAroundX(r32 deg, Vec3 *points, int num_points) {
	r32 rad = DEG2RAD(-deg);
   r32 m[2][3] = 
   {
      { 0.0f, (r32)cos(rad), (r32)sin(rad) }, 
      { 0.0f, -m[0][2], m[0][1]  }
   };

	for (int i = 0; i < num_points; ++i) {
		r32 y = Dot3(m[0], points[i]);
		r32 z = Dot3(m[1], points[i]);

		points[i][1] = y;
		points[i][2] = z;
	}
}

void RotateAroundX(r32 deg, PolyVert *points, int num_points) {
	r32 rad = DEG2RAD(-deg);
   r32 m[2][3] = 
   {
      { 0.0f, (r32)cos(rad), (r32)sin(rad) }, 
      { 0.0f, -m[0][2], m[0][1]  }
   };

	for (int i = 0; i < num_points; ++i) {
		r32 y = Dot3(m[0], points[i].xyz);
		r32 z = Dot3(m[1], points[i].xyz);

		points[i].xyz[1] = y;
		points[i].xyz[2] = z;
	}
}

void RotateAroundY(r32 deg, Vec3 *points, int num_points) {
	r32 rad = DEG2RAD(-deg);
   r32 m[2][3] = 
   {
      { (r32)cos(rad), 0.0f, (r32)-sin(rad) }, 
      { -m[0][2], 0.0f,  m[0][0] }
   };

	for (int i = 0; i < num_points; ++i) {
		r32 x = Dot3(m[0], points[i]);
		r32 z = Dot3(m[1], points[i]);

		points[i][0] = x;
		points[i][2] = z;
	}
}

void RotateAroundY(r32 deg, PolyVert *points, int num_points) {
	r32 rad = DEG2RAD(-deg);
   r32 m[2][3] = 
   {
      { (r32)cos(rad), 0.0f, (r32)-sin(rad) }, 
      { -m[0][2], 0.0f,  m[0][0] }
   };

	for (int i = 0; i < num_points; ++i) {
		r32 x = Dot3(m[0], points[i].xyz);
		r32 z = Dot3(m[1], points[i].xyz);

		points[i].xyz[0] = x;
		points[i].xyz[2] = z;
	}
}

void RotateAroundZ(r32 deg, Vec3 *points, int num_points) {
	r32 rad = DEG2RAD(-deg);
   r32 m[2][3] = 
   {
      { (r32)cos(rad), (r32)sin(rad), 0.0f }, 
      { -m[0][1], m[0][0],  0.0f }
   };

	for (int i = 0; i < num_points; ++i) {
		r32 x = Dot3(m[0], points[i]);
		r32 y = Dot3(m[1], points[i]);

		points[i][0] = x;
		points[i][1] = y;
	}
}

void RotateAroundZ(r32 deg, PolyVert *points, int num_points) {
	r32 rad = DEG2RAD(-deg);
   r32 m[2][3] = 
   {
      { (r32)cos(rad), (r32)sin(rad), 0.0f }, 
      { -m[0][1], m[0][0],  0.0f }
   };

	for (int i = 0; i < num_points; ++i) {
		r32 x = Dot3(m[0], points[i].xyz);
		r32 y = Dot3(m[1], points[i].xyz);

		points[i].xyz[0] = x;
		points[i].xyz[1] = y;
	}
}

#if 0
// culling in view space
void RF_CullBackFaces(ViewSystem *vs, Poly *polys, int num_polys) {
	Vec3 p = {0.0f, 0.0f, 0.0f};

	for (int i = 0; i < num_polys; ++i) {
		if (polys[i].state & POLY_STATE_BACKFACE) {
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
#endif

static void RF_RotateForViewer(ViewSystem *vs) {
	r32	view_matrix[16];
	Vec3	origin;

	Vec3Copy(origin, vs->world_orientation.origin);

	// compute the uvn vectors in view space(LH)
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

void RF_UpdateView(ViewSystem *vs) {
	RF_RotateForViewer(vs);
	RF_ExtractWorldPlanes(vs);					
   RF_ExtractViewPlanes(vs);
}

#if 0
static void RF_AddPoly(RendererBackend *rb, PolyVert v0, PolyVert v1, PolyVert v2) {
	const int num_verts = 3;	// triangle
   Poly *poly = &rb->polys[rb->num_polys];
   poly->color = MV4(1.0f, 1.0f, 1.0f, 1.0f);
   poly->num_verts = num_verts;
   poly->vertex_array = &rb->poly_verts[rb->num_poly_verts];
   poly->vertex_array[0] = v0;
   poly->vertex_array[1] = v1;
   poly->vertex_array[2] = v2;
   rb->num_polys++;
   rb->num_poly_verts += num_verts;
}
#endif

static void RF_AddPoly(RendererBackend *rb, Poly vis_poly) {
	const int num_verts = 3;	// triangle
	Poly *poly = &rb->vis_polys[rb->vis_num_polys];
   poly->vertex_array = &rb->vis_poly_verts[rb->vis_num_poly_verts];
   poly->vertex_array[0] = vis_poly.vertex_array[0];
   poly->vertex_array[1] = vis_poly.vertex_array[1];
   poly->vertex_array[2] = vis_poly.vertex_array[2];
   poly->num_verts = num_verts;
   poly->color = vis_poly.color;
   rb->vis_num_polys++;
   rb->vis_num_poly_verts += num_verts;
}

static void RF_AddClippedPoly(RendererBackend *rb, PolyVert v0, PolyVert v1, PolyVert v2) {
	const int num_verts = 3;	// triangle
   Poly *poly = &rb->vis_polys[rb->vis_num_polys];
   poly->color = MV4(1.0f, 1.0f, 1.0f, 1.0f);
   poly->num_verts = num_verts;
   poly->vertex_array = &rb->vis_poly_verts[rb->vis_num_poly_verts];
   poly->vertex_array[0] = v0;
   poly->vertex_array[1] = v1;
   poly->vertex_array[2] = v2;
   rb->vis_num_polys++;
   rb->vis_num_poly_verts += num_verts;
}

void RF_AddPolys(RendererBackend *rb, const PolyVert *verts, const u16 (*index_list)[3], int num_polys) {
	const int num_verts = 3;	// triangle
	Assert(rb->num_polys + num_polys <= MAX_NUM_POLYS);

	for (int i = 0; i < num_polys; i++) {
		Poly *poly = &rb->polys[rb->num_polys];

		poly->num_verts = num_verts;
		poly->vertex_array = &rb->poly_verts[rb->num_poly_verts];

		poly->vertex_array[0] = verts[index_list[i][0]];
		poly->vertex_array[1] = verts[index_list[i][1]];
		poly->vertex_array[2] = verts[index_list[i][2]];

		rb->num_polys++; 
		rb->num_poly_verts += 3;
	}
}

void RF_AddCubePolys(RendererBackend *rb, ViewSystem *vs, const PolyVert *verts, const u16 (*index_list)[3], int num_polys, r32 texture_scale) {
	const s32 num_verts = 3;	// triangle
	Assert(rb->num_polys + num_polys <= MAX_NUM_POLYS);
	Assert(texture_scale > 0.0f && texture_scale <= 1.0f);

	for (int i = 0; i < num_polys; i += 2) {
		Poly *poly1;
		Poly *poly2;
      Poly tmp1 = {};
		Poly tmp2 = {};
      PolyVert pv_buf[6];

      tmp1.color = MV4(1.0f, 1.0f, 1.0f, 1.0f);     // test base color
		tmp1.num_verts = num_verts;
		tmp1.vertex_array = &pv_buf[0];

      tmp2.color = MV4(1.0f, 1.0f, 1.0f, 1.0f);     // test base color
		tmp2.num_verts = num_verts;
		tmp2.vertex_array = &pv_buf[num_verts];

		tmp1.vertex_array[0] = verts[index_list[i][0]];
		tmp1.vertex_array[1] = verts[index_list[i][1]];
		tmp1.vertex_array[2] = verts[index_list[i][2]];

		tmp2.vertex_array[0] = verts[index_list[i+1][0]];
		tmp2.vertex_array[1] = verts[index_list[i+1][1]];
		tmp2.vertex_array[2] = verts[index_list[i+1][2]];

		tmp1.vertex_array[0].uv[0] = 0.0f*texture_scale;
		tmp1.vertex_array[0].uv[1] = 0.0f*texture_scale;
		tmp1.vertex_array[1].uv[0] = 1.0f*texture_scale;
		tmp1.vertex_array[1].uv[1] = 0.0f*texture_scale;
		tmp1.vertex_array[2].uv[0] = 1.0f*texture_scale;
		tmp1.vertex_array[2].uv[1] = 1.0f*texture_scale;

		tmp2.vertex_array[0].uv[0] = 0.0f*texture_scale;
		tmp2.vertex_array[0].uv[1] = 0.0f*texture_scale;
		tmp2.vertex_array[1].uv[0] = 1.0f*texture_scale;
		tmp2.vertex_array[1].uv[1] = 1.0f*texture_scale;
		tmp2.vertex_array[2].uv[0] = 0.0f*texture_scale;
		tmp2.vertex_array[2].uv[1] = 1.0f*texture_scale;

      if (RF_CullPoly(vs, &tmp1) == CULL_IN) {
		   poly1 = &rb->polys[rb->num_polys];
         poly1->color = tmp1.color;
         poly1->num_verts = tmp1.num_verts;
         poly1->vertex_array = &rb->poly_verts[rb->num_poly_verts];
         poly1->vertex_array[0] = tmp1.vertex_array[0];
         poly1->vertex_array[1] = tmp1.vertex_array[1];
         poly1->vertex_array[2] = tmp1.vertex_array[2];
         rb->num_polys++;
         rb->num_poly_verts += num_verts;
      } 
      if (RF_CullPoly(vs, &tmp2) == CULL_IN) {
		   poly2 = &rb->polys[rb->num_polys];
         poly2->color = tmp2.color;
         poly2->num_verts = tmp2.num_verts;
         poly2->vertex_array = &rb->poly_verts[rb->num_poly_verts];
         poly2->vertex_array[0] = tmp2.vertex_array[0];
         poly2->vertex_array[1] = tmp2.vertex_array[1];
         poly2->vertex_array[2] = tmp2.vertex_array[2];
         rb->num_polys++;
         rb->num_poly_verts += num_verts;
      } 
	}
}

void RF_CalculateVertexNormals(Poly *polys, int num_polys, PolyVert *poly_verts, int num_poly_verts) {
	for (int i = 0; i < num_poly_verts; i++) {
		poly_verts[i].normal[0] = 0.0f;
		poly_verts[i].normal[1] = 0.0f;
		poly_verts[i].normal[2] = 0.0f;
	}
	// FIXME: function to compute poly normals
	for (int i = 0; i < num_polys; i++) {
		PolyVert v0 = polys[i].vertex_array[0];
		PolyVert v1 = polys[i].vertex_array[1];
		PolyVert v2 = polys[i].vertex_array[2];
		Vec3 u = MakeVec3(v0.xyz, v1.xyz);
		Vec3 v = MakeVec3(v0.xyz, v2.xyz);
		Vec3 n = -Cross3(u, v);					// NOTE: negated because of left-handed system and because we are using ccw winding order
		v0.normal = v0.normal + n;
		v1.normal = v1.normal + n;
		v2.normal = v2.normal + n;
		polys[i].vertex_array[0].normal = v0.normal;
		polys[i].vertex_array[1].normal = v1.normal;
		polys[i].vertex_array[2].normal = v2.normal;
	}
	for (int i = 0; i < num_poly_verts; i++) {
		poly_verts[i].normal = Vec3Norm(poly_verts[i].normal);
	}
}

static void ClipPoly(Renderer *r, r32 p0, r32 p1, r32 p2, PolyVert pv0, PolyVert pv1, PolyVert pv2, Vec3 n, r32 d) {
   Assert(p0*p1 < 0.0f);

   Vec3 v = pv1.xyz - pv0.xyz;
   Vec2 tex_v = pv1.uv_vec - pv0.uv_vec;
   r32 denom = Dot3(n, v);
   Assert(denom != 0.0f);
   r32 t01 = (d - Dot3(n, pv0.xyz)) / denom;
   Assert(t01 <= 1.0f && t01 > 0.0f);
   Vec3 vc01 = pv0.xyz + v*t01;
   PolyVert pvc01 = {};
   pvc01.xyz = vc01;
   pvc01.uv_vec = pv0.uv_vec + tex_v*t01;


   if (p0 > 0.0f) {                 // p1 < 0.0f
      if (p2 > 0.0f) {
         Vec3 v = pv2.xyz - pv1.xyz;
         Vec2 tex_v = pv2.uv_vec - pv1.uv_vec;
         r32 denom = Dot3(n, v);
         Assert(denom != 0.0f);
         r32 t12 = (d - Dot3(n, pv1.xyz)) / denom;
         Assert(t12 <= 1.0f && t12 > 0.0f);
         Vec3 vc12 = pv1.xyz + v*t12;
         PolyVert pvc12 = {};
         pvc12.xyz = vc12;
         pvc12.uv_vec = pv1.uv_vec + tex_v*t12;

         PolyVert ppv0 = pv0;
         PolyVert ppv1 = pvc01;
         PolyVert ppv2 = pvc12;
         PolyVert ppv3 = pv2;
         RF_AddClippedPoly(&r->back_end, ppv0, ppv1, ppv2);
         RF_AddClippedPoly(&r->back_end, ppv0, ppv2, ppv3);
      } else {
         Vec3 v = pv2.xyz - pv0.xyz;
         Vec2 tex_v = pv2.uv_vec - pv0.uv_vec;
         r32 denom = Dot3(n, v);
         Assert(denom != 0.0f);
         r32 t02 = (d - Dot3(n, pv0.xyz)) / denom;
         Assert(t02 <= 1.0f && t02 > 0.0f);
         Vec3 vc02 = pv0.xyz + v*t02;
         PolyVert pvc02 = {};
         pvc02.xyz = vc02;
         pvc02.uv_vec = pv0.uv_vec + tex_v*t02;

         PolyVert ppv0 = pv0;
         PolyVert ppv1 = pvc01;
         PolyVert ppv2 = pvc02;
         RF_AddClippedPoly(&r->back_end, ppv0, ppv1, ppv2);
      }
   } else {                         // p0 < 0.0f
      if (p2 > 0.0f) {
         Vec3 v = pv0.xyz - pv2.xyz;
         Vec2 tex_v = pv0.uv_vec - pv2.uv_vec;
         r32 denom = Dot3(n, v);
         Assert(denom != 0.0f);
         r32 t20 = (d - Dot3(n, pv2.xyz)) / denom;
         Assert(t20 <= 1.0f && t20 > 0.0f);
         Vec3 vc20 = pv2.xyz + v*t20;
         PolyVert pvc20 = {};
         pvc20.xyz = vc20;
         pvc20.uv_vec = pv2.uv_vec + tex_v*t20;

         PolyVert ppv0 = pv2;
         PolyVert ppv1 = pvc20;
         PolyVert ppv2 = pvc01;
         PolyVert ppv3 = pv1;
         RF_AddClippedPoly(&r->back_end, ppv0, ppv1, ppv2);
         RF_AddClippedPoly(&r->back_end, ppv0, ppv2, ppv3);
      } else {
         Vec3 v = pv2.xyz - pv1.xyz;
         Vec2 tex_v = pv2.uv_vec - pv1.uv_vec;
         r32 denom = Dot3(n, v);
         Assert(denom != 0.0f);
         r32 t12 = (d - Dot3(n, pv1.xyz)) / denom;
         Assert(t12 <= 1.0f && t12 > 0.0f);
         Vec3 vc12 = pv1.xyz + v*t12;
         PolyVert pvc12 = {};
         pvc12.xyz = vc12;
         pvc12.uv_vec = pv1.uv_vec + tex_v*t12;

         PolyVert ppv0 = pv1;
         PolyVert ppv1 = pvc12;
         PolyVert ppv2 = pvc01;
         RF_AddClippedPoly(&r->back_end, ppv0, ppv1, ppv2);
      }
   }
}

// clipping in viewspace
void RF_Clip(Renderer *r) {
   int num_polys = r->back_end.num_polys;
   Assert(r->back_end.vis_num_polys == 0);
   Assert(r->back_end.vis_num_poly_verts == 0);
   for (int i = 0; i < num_polys; i++) {
      b32 inside = true;
      Poly *poly = &r->back_end.polys[i];
      PolyVert pv0 = poly->vertex_array[0];
      PolyVert pv1 = poly->vertex_array[1];
      PolyVert pv2 = poly->vertex_array[2];
      // clipping to near-z only atm
      for (int j = 4; j < NUM_FRUSTUM_PLANES; j++) {
         Vec3 n = r->front_end.current_view.view_planes[j].unit_normal;
         r32 d = r->front_end.current_view.view_planes[j].dist;
         r32 p0 = Dot3(n, pv0.xyz) - d;
         r32 p1 = Dot3(n, pv1.xyz) - d;
         r32 p2 = Dot3(n, pv2.xyz) - d;
         if (p0*p1 < 0.0f) {
            ClipPoly(r, p0, p1, p2, pv0, pv1, pv2, n, d);
            inside = false;
         } else if (p1*p2 < 0.0f) {
            ClipPoly(r, p1, p2, p0, pv1, pv2, pv0, n, d);
            inside = false;
         } else if (p2*p0 < 0.0f) {
            ClipPoly(r, p2, p0, p1, pv2, pv0, pv1, n, d);
            inside = false;
         } 
      }
      if (inside) {
         RF_AddPoly(&r->back_end, *poly);
      }
   }
}
