#include "r_types.h"
#include "renderer.h"

// FIXME: move this into r_cmds.cc
void R_AddPolys(RendererBackend *rb, const Vec3 *verts, Poly *poly_array, int num_verts, int num_polys) {
	Poly *poly;
	Assert(rb->num_polys + num_polys <= MAX_NUM_POLYS);

	for (int i = 0; i < num_polys; ++i) {
		poly = &rb->polys[rb->num_polys];
		poly->num_verts = num_verts;
		poly->state = poly_array[i].state;
		poly->color = poly_array[i].color;
		if (poly->color != 0xff) {
			int x = 42;
		}
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
