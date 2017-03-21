#include "renderer.h"
#include "win_renderer.h"
#include "plg_loader.h"

void R_SetupProjection(Renderer *ren) {
	r32 aspect_ratio = ren->current_view.aspect_ratio;
	r32 fov_y = ren->current_view.fov_y;

	// NOTE: currently this handles only view planes 2x2 dimension
	// FIXME: handle variable sized view planes
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

// Gribb & Hartmann method
void R_SetupFrustum(Renderer *ren) {
	Plane frustum[4];
	r32	combo_matrix[4][4];
	Mat4x4Mul(combo_matrix, ren->current_view.view_matrix, ren->current_view.projection_matrix);

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

	// FIXME: add near and far planes

	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		frustum[i].unit_normal = Vector3Normalize(frustum[i].unit_normal);
		frustum[i].dist = Vec3Dot(frustum[i].unit_normal, ren->current_view.world_orientation.origin);
	}

	memcpy(&ren->current_view.frustum, &frustum, sizeof(frustum));
}

void R_TransformWorldToView(Renderer *ren, Entity *md) {
	int num_verts = md->status.num_verts;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;
	for (int i = 0; i < num_verts; ++i) {
		r32 vert[4], tmp[4];
		Vec3Copy(vert, trans_verts[i]);
		vert[3] = 1.0f;

		Mat1x4Mul(tmp, vert, ren->current_view.view_matrix);  
		Vec3Copy(trans_verts[i], tmp);
	}
}

void R_TransformModelToWorld(Renderer *ren, Entity *md, VertexTransformState vts) {
	int num_verts = md->status.num_verts;
	Vec3 *local_verts = md->mesh->local_verts->vert_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	if (vts == VTS_LOCAL_TO_TRANSFORMED) {
		for (int i = 0; i < num_verts; ++i) {
			Vector3Add(local_verts[i], md->status.world_pos, trans_verts[i]);
		}
	} else if (vts == VTS_TRANSFORMED_ONLY) {
		for (int i = 0; i < num_verts; ++i) {
			Vector3Add(trans_verts[i], md->status.world_pos, trans_verts[i]);
		}
	}
}

FrustumClippingState R_CullPointAndRadius(Renderer *ren, Vec3 pt, r32 radius) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; ++i) {
		Plane pl = ren->current_view.frustum[i];
		r32 dist = Vec3Dot(pt, pl.unit_normal) - pl.dist;

		if (dist < 0.0f) {
			//Sys_Print("Culling!!\n");
			return FCS_CULL_OUT;
			
		} 
	}

	//Sys_Print("NO culling!!\n");
	return FCS_CULL_IN;
}

void R_TransformViewToClip(Renderer *ren, Entity *md) {
	int num_verts = md->status.num_verts;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;
	r32 (*m)[4] = ren->current_view.projection_matrix;
	r32 in[4];
	r32 out[4];

	for (int i = 0; i < num_verts; ++i) {
		in[0] = trans_verts[i][0];
		in[1] = trans_verts[i][1];
		in[2] = trans_verts[i][2];
		in[3] = 1.0f;

		Mat1x4Mul(out, in, m);  
		trans_verts[i][0] = out[0] / out[3];
		trans_verts[i][1] = out[1] / out[3];
		trans_verts[i][2] = out[2] / out[3];
	}
}

void R_TransformClipToScreen(Renderer *ren, Entity *md) {
	int num_verts = md->status.num_verts;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	r32 screen_width_factor = (0.5f * ren->current_view.viewport_width) - 0.5f;
	r32 screen_height_factor = (0.5f * ren->current_view.viewport_height) - 0.5f;
	for (int i = 0; i < num_verts; ++i) {
		trans_verts[i][0] = screen_width_factor + (trans_verts[i][0] * screen_width_factor);
		trans_verts[i][1] = screen_height_factor - (trans_verts[i][1] * screen_height_factor);
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

void R_CullBackFaces(Renderer *ren, Entity *md) {
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

		r32 dot = Vec3Dot(view, n);

		if (dot <= 0.0f) {
			polys[i].state |= POLY_STATE_BACKFACE;
		}
	}
}

void R_RotateForViewer(Renderer *ren) {
	r32		view_matrix[16];
	Vec3	origin;

	Vec3Copy(origin, ren->current_view.world_orientation.origin);

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

	memcpy(ren->current_view.view_matrix, view_matrix, sizeof(view_matrix));
}

void R_RenderView(Renderer *ren) {
	static b32 first_draw = false;
	if (!first_draw) {
		first_draw = true;

		Vec3Init(ren->current_view.world_orientation.origin, 0.0f, 0.0f, 0.0f);
		Vec3Init(ren->current_view.target, 0.0f, 0.0f, 1.0f);

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
	R_SetupProjection(ren);
	R_SetupFrustum(ren);					


	//R_GenerateDrawSurfs();

	//R_SortDrawSurfs( tr.refdef.drawSurfs + firstDrawSurf, tr.refdef.numDrawSurfs - firstDrawSurf );

	//// draw main system development information (surface outlines, etc)
	//R_DebugGraphics();
}
