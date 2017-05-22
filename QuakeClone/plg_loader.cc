#include "plg_loader.h"

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
#define IsEmptyChar(c) ((char)(c) == 10 || (c) == ' ' || (c) == '\t') 
#define IsHashTag(c) ((c) == '#') 
#define IsNewLine(c) ((c) == '\n') 

static ptrdiff_t PLG_ReadLine(char *buffer, const void *load_data, u32 load_data_len, ptrdiff_t load_data_index) {
	Assert(load_data);
	char *base = (char *)load_data;
	char *src = base + load_data_index;

	while (IsHashTag(*src)) {
		while ((ptrdiff_t)(src - base) <  (ptrdiff_t)load_data_len && !IsNewLine(*src++)) ;
	} 
	do {
		if (IsNewLine(*src)) {
			break;
		}
		if (!IsEmptyChar(*src)) {
			*buffer++ = *src;
		} else {
			*buffer++ = ' ';
		}
	} while ((src++ - base) <  (ptrdiff_t)load_data_len);

	Assert((ptrdiff_t)(src - base) <  (ptrdiff_t)load_data_len);
	return ((ptrdiff_t)(src - base)) + 1;
}

// FIXME: check for sscanf_s over/underflows
b32 PLG_LoadMesh(Entity *typeless_ent, const void *load_data, u32 load_data_len, r32 scale) {
	char buffer[MAX_PLG_LINE_LEN];
	int local_verts_offset = 0;
	int polys_offset = 0;
	ptrdiff_t load_data_index = 0;

	// FIXME: add proper checkings!!!
	// check the entity types

	//typeless_ent->status.state = POLY_STATE_ACTIVE | POLY_STATE_VISIBLE;

	// get the object description
	if ((load_data_index = PLG_ReadLine(buffer, load_data, load_data_len, load_data_index)) == 1) {
		Sys_Print("\nError while reading lines from an opened PLG file, it should be a name of the mesh to be loaded,"
				  "number of verts and number polys");
		return false;
	}

	if (typeless_ent->type_enum == EntityType_player) {
		Assert(sizeof(typeless_ent->status.num_verts) == sizeof(s16));
		Assert(sizeof(typeless_ent->status.num_polys) == sizeof(s16));
		sscanf_s(buffer, "%s %hd %hd", typeless_ent->status.type_name, sizeof(typeless_ent->status.type_name), &typeless_ent->status.num_verts, &typeless_ent->status.num_polys);
		memset(buffer, 0, sizeof(buffer));
		local_verts_offset = OffsetOf(player.local_vertex_array, Entity);

		polys_offset = OffsetOf(player.polys, Entity);
	} else {
		Assert(sizeof(typeless_ent->status.num_verts) == sizeof(s16));
		Assert(sizeof(typeless_ent->status.num_polys) == sizeof(s16));
		sscanf_s(buffer, "%s %hd %hd", typeless_ent->status.type_name, sizeof(typeless_ent->status.type_name), &typeless_ent->status.num_verts, &typeless_ent->status.num_polys);
		memset(buffer, 0, sizeof(buffer));

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
		if ((load_data_index = PLG_ReadLine(buffer, load_data, load_data_len, load_data_index)) == 1) {
			Sys_Print("\nError while reading lines from an opened PLG file, it should be a vertex in the x y z order");
			return false;
		}

		sscanf_s(buffer, "%f %f %f",
				 &local_vertex_array[i][0],
				 &local_vertex_array[i][1],
				 &local_vertex_array[i][2]);
		memset(buffer, 0, sizeof(buffer));

		// NOTE: convert from ccw into cw vertex winding order for our left-handed system
		r32 v0 = local_vertex_array[i][0];
		r32 v2 = local_vertex_array[i][2];

		local_vertex_array[i][0] = v2;
		local_vertex_array[i][2] = v0;
	}

	int poly_num_verts = 0;

	u32 poly_surface_desc = 0;
	char tmp_poly_surface_desc[10 + 1];		// one for the null terminator

	Poly *poly_array = (Poly *)((byte *)typeless_ent + polys_offset);  
	int num_polys = typeless_ent->status.num_polys;
	for (int i = 0; i < num_polys; ++i) {
		if ((load_data_index = PLG_ReadLine(buffer, load_data, load_data_len, load_data_index)) == 1) {
			Sys_Print("\nError while reading lines from an opened PLG file," 
					  "it should be a 32 bit value in the form of PLG/X format: AA | RR| GG | BB");
			return false;
		}

		Assert(sizeof(*poly_array[i].vert_indices) == sizeof(u16));
		sscanf_s(buffer, "%s %u %hu %hu %hu", 
				 tmp_poly_surface_desc,
				 sizeof(tmp_poly_surface_desc),
				 &poly_surface_desc,
				 &poly_array[i].vert_indices[0],
				 &poly_array[i].vert_indices[1],
				 &poly_array[i].vert_indices[2]);
		memset(buffer, 0, sizeof(buffer));
	
		if (tmp_poly_surface_desc[0] == '0' && tmp_poly_surface_desc[1] == 'x') {
			sscanf_s(tmp_poly_surface_desc, "%x", &poly_surface_desc);
			memset(buffer, 0, sizeof(buffer));
		} else {
			poly_surface_desc = atoi(tmp_poly_surface_desc);
		}

		if (poly_surface_desc & PLX_2SIDED_FLAG) {
			//ent->poly_array[i].attr |= POLY_ATTR_2SIDED;
		}

		u32 alpha = (poly_surface_desc & 0xff000000) >> 24u;
		u32 red = (poly_surface_desc   & 0xff0000) >> 16u;
		u32 green = (poly_surface_desc & 0xff00) >> 8u;
		u32 blue = (poly_surface_desc  & 0xff) >> 0u;

		poly_array[i].color = RGB_32(alpha, red, green, blue);

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
	return true;
}

