#include "plg_loader.h"

//	PLG/X format: CSSD | RRRR| GGGG | BBBB

//	C	= 1 for 24-bit, 0 for 8-bit color
//	SS	= Shading mode used
//	D	= Double-sided bit, 1 for double sided

//	bit masks to simplify testing
#define PLX_RGB_MASK		0x8000		// mask extracts RGB/indexed color
#define PLX_SHADE_MODE_MASK 0x6000		// mask extracts shading mode
#define PLX_2SIDED_MASK		0x1000		// mask for double sided
#define PLX_COLOR_MASK		0x0fff		// xxxxrrrrggggbbbb, 4-bits per channel RGB

//	double sided flag
#define PLX_2SIDED_FLAG	0x1000			// poly is double sided
#define PLX_1SIDED_FLAG	0x0000			// poly is single sided

// these are the comparison flags after masking
// color mode of polygon
//#define PLX_COLOR_MODE_8BIT_FLAG	0x8000		// poly uses indexed 8-bit color

//	shading mode of polygon
#define PLX_SHADE_MODE_PURE_FLAG	0x0000	// poly is a constant color
#define PLX_SHADE_MODE_FLAT_FLAG	0x2000	// poly uses flat shading
#define PLX_SHADE_MODE_GOURAUD_FLAG 0x4000	// poly used gouraud shading
#define PLX_SHADE_MODE_PHONG_FLAG	0x6000	// poly uses phong shading


#define MAX_PLG_LINE_LEN 256
#define IsEmptyChar(c) ((char)(c) == 10 || (c) == ' ' || (c) == '\n' || (c) == '\t') 

static const char *PLG_ParseLine(char *buffer, int max_len, FILE *fp) {
	int i;
	size_t len;

	for (;;) {
		if (!fgets(buffer, max_len, fp)) {
			return 0;
		}

		for (len = strlen(buffer), i = 0; 
			(IsEmptyChar(buffer[i])); ++i) 
			;

		if (i >= len || buffer[i] == '#') {
			continue;
		}

		return buffer + i;
	}
}

// NOTE: this parser routine is just for prototyping, aka ignoring all overflows atm
b32 PLG_LoadMesh(Entity *typeless_ent, FILE **fp, r32 scale) {
	char buffer[MAX_PLG_LINE_LEN];
	const char *parsed_string;
	// FIXME: add proper checkings!!!
	// check the entity types
	int local_verts_offset = 0;
	int polys_offset = 0;

	//typeless_ent->status.state = POLY_STATE_ACTIVE | POLY_STATE_VISIBLE;

	// get the object description
	if (!(parsed_string = PLG_ParseLine(buffer, MAX_PLG_LINE_LEN - 1, *fp))) {
		Sys_Print("\nError while reading lines from an opened PLG file, it should be a name of the mesh to be loaded,"
				  "number of verts and number polys");
		return false;
	}

	if (typeless_ent->type_enum == EntityType_player) {
		sscanf_s(parsed_string, "%s %d %d", typeless_ent->status.type_name, sizeof(typeless_ent->status.type_name), &typeless_ent->status.num_verts, &typeless_ent->status.num_polys);
		local_verts_offset = OffsetOf(player.local_vertex_array, Entity);

		polys_offset = OffsetOf(player.polys, Entity);
	} else {
		sscanf_s(parsed_string, "%s %d %d", typeless_ent->status.type_name, sizeof(typeless_ent->status.type_name), &typeless_ent->status.num_verts, &typeless_ent->status.num_polys);

		if (StrCmp(typeless_ent->status.type_name, global_entity_names[EntityType_cube]) == 0) {
			typeless_ent->type_enum = EntityType_cube;
			local_verts_offset = OffsetOf(cube.local_vertex_array, Entity);

			polys_offset = OffsetOf(cube.polys, Entity);
		} else {
			InvalidCodePath("Unhandled entitity type!");
		}
	}
	Vec3 *local_vertex_array = (Vec3 *)((byte *)typeless_ent + local_verts_offset);  
	int num_verts = typeless_ent->status.num_verts;

	for (int i = 0; i < num_verts; ++i) {
		if (!(parsed_string = PLG_ParseLine(buffer, MAX_PLG_LINE_LEN - 1, *fp))) {
			Sys_Print("\nError while reading lines from an opened PLG file, it should be a vertex in the x y z order");
			return false;
		}

		sscanf_s(parsed_string, "%f %f %f",
				 &local_vertex_array[i][0],
				 &local_vertex_array[i][1],
				 &local_vertex_array[i][2]);

		// NOTE: convert from ccw into cw vertex winding order for our left-handed system
		r32 v0 = local_vertex_array[i][0];
		r32 v2 = local_vertex_array[i][2];

		local_vertex_array[i][0] = v2;
		local_vertex_array[i][2] = v0;
	}


	// FIXME: compute the avg and max radius for mesh object here

	int poly_num_verts = 0;

	u32 poly_surface_desc = 0;
	char tmp_poly_surface_desc[10 + 1];		// one for the null terminator

	Poly *poly_array = (Poly *)((byte *)typeless_ent + polys_offset);  
	int num_polys = typeless_ent->status.num_polys;
	for (int i = 0; i < num_polys; ++i) {
		if (!(parsed_string = PLG_ParseLine(buffer, MAX_PLG_LINE_LEN - 1, *fp))) {
			Sys_Print("\nError while reading lines from an opened PLG file," 
					  "it should be a 32 bit value in the form of PLG/X format: AA | RR| GG | BB");
			return false;
		}

		sscanf_s(parsed_string, "%s %u %d %d %d", 
				 tmp_poly_surface_desc,
				 sizeof(tmp_poly_surface_desc),
				 &poly_surface_desc,
				 &poly_array[i].vert_indices[0],
				 &poly_array[i].vert_indices[1],
				 &poly_array[i].vert_indices[2]);
	
		if (tmp_poly_surface_desc[0] == '0' && tmp_poly_surface_desc[1] == 'x') {
			sscanf_s(tmp_poly_surface_desc, "%x", &poly_surface_desc);
		} else {
			poly_surface_desc = atoi(tmp_poly_surface_desc);
		}

		if (poly_surface_desc & PLX_2SIDED_FLAG) {
			//ent->poly_array[i].attr |= POLY_ATTR_2SIDED;
		}

		//if (poly_surface_desc & PLX_COLOR_MODE_8BIT_FLAG) {
		//	poly_array[i].color = poly_surface_desc & 0x00ff;
		//	//poly_array[i].color |= POLY_ATTR_8BITCOLOR;
		//} else {
			//poly_array[i].color |= POLY_ATTR_RGB24;
		u32 alpha = (poly_surface_desc & 0xff000000) >> 24u;
		u32 red = (poly_surface_desc   & 0xff0000) >> 16u;
		u32 green = (poly_surface_desc & 0xff00) >> 8u;
		u32 blue = (poly_surface_desc  & 0xff) >> 0u;

		poly_array[i].color = RGB_32(alpha, red, green, blue);
		//}

	//	int shade_mode = poly_surface_desc & PLX_SHADE_MODE_MASK;

	//	switch (shade_mode) {
	//		case PLX_SHADE_MODE_PURE_FLAG: {
	//			//mo->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_PURE;
	//		} break;
	//		case PLX_SHADE_MODE_FLAT_FLAG: {
	//			//mo->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_FLAT;
	//		} break;
	//		case PLX_SHADE_MODE_GOURAUD_FLAG: {
	//			//mo->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_GOURAUD;
	//		} break;
	//		case PLX_SHADE_MODE_PHONG_FLAG: {
	//			//mo->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_PHONG;
	//		} break;
	//		default: {
	//			Sys_Print("\nShading mode is invalid, it should be 1 of: 0x0000, 0x2000, 0x4000, 0x6000\n");
	//		}
	//	}

		poly_array[i].state = POLY_STATE_ACTIVE;
	}

	Sys_Print("\nMesh data loading complete\n");
	fseek(*fp, 0, SEEK_SET);
	return true;
}

