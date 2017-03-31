#include "r_types.h"
#include "renderer.h"

void R_AddPolys(RendererBackend *rb, const Vec3 *verts, Poly *poly_array, int num_verts, int num_polys) {
	Poly *poly;
	Assert(rb->num_polys + num_polys <= MAX_NUM_POLYS);

	for (int i = 0; i < num_polys; ++i) {
		poly = &rb->polys[rb->num_polys];
		poly->num_verts = num_verts;
		poly->state = poly_array[i].state;
		poly->color = poly_array[i].color;
		poly->vertex_array = &rb->poly_verts[rb->num_verts];

		for (int j = 0; j < num_verts; ++j) {
			poly->vertex_array[j] = verts[poly_array[i].vert_indices[j]];
		}

		++rb->num_polys; 
		rb->num_verts += num_verts;
	}
		
#if 0
		Com_Memcpy( poly->verts, &verts[numVerts*j], numVerts * sizeof( *verts ) );

		if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
			poly->verts->modulate[0] = 255;
			poly->verts->modulate[1] = 255;
			poly->verts->modulate[2] = 255;
			poly->verts->modulate[3] = 255;
		}
		// done.
		r_numpolys++;
		r_numpolyverts += numVerts;

		// if no world is loaded
		if ( tr.world == NULL ) {
			fogIndex = 0;
		}
		// see if it is in a fog volume
		else if ( tr.world->numfogs == 1 ) {
			fogIndex = 0;
		} else {
			// find which fog volume the poly is in
			VectorCopy( poly->verts[0].xyz, bounds[0] );
			VectorCopy( poly->verts[0].xyz, bounds[1] );
			for ( i = 1 ; i < poly->numVerts ; i++ ) {
				AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
			}
			for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
				fog = &tr.world->fogs[fogIndex]; 
				if ( bounds[1][0] >= fog->bounds[0][0]
					&& bounds[1][1] >= fog->bounds[0][1]
					&& bounds[1][2] >= fog->bounds[0][2]
					&& bounds[0][0] <= fog->bounds[1][0]
					&& bounds[0][1] <= fog->bounds[1][1]
					&& bounds[0][2] <= fog->bounds[1][2] ) {
					break;
				}
			}
			if ( fogIndex == tr.world->numfogs ) {
				fogIndex = 0;
			}
		}
		poly->fogIndex = fogIndex;
	}
#endif
}
