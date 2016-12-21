#include "plg_loader.h"

//	PLG/X format: CSSD | RRRR| GGGG | BBBB

//	C	= 1 for 16-bit, 0 for 8-bit color
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
#define PLX_COLOR_MODE_RGB_FLAG		0x8000		// poly uses RGB color
#define PLX_COLOR_MODE_INDEXED_FLAG	0x0000	// poly uses indexed 8-bit color

//	shading mode of polygon
#define PLX_SHADE_MODE_PURE_FLAG	0x0000	// poly is a constant color
#define PLX_SHADE_MODE_FLAT_FLAG	0x2000	// poly uses flat shading
#define PLX_SHADE_MODE_GOURAUD_FLAG 0x4000	// poly used gouraud shading
#define PLX_SHADE_MODE_PHONG_FLAG	0x6000	// poly uses phong shading


#define MAX_PLG_LINE_LEN 256
#define IS_EMPTY_CHAR(c) ((char)(c) == 10 || (c) == ' ' || (c) == '\n' || (c) == '\t') 
static const char *PLG_ParseLine(char *buffer, int max_len, FILE *fp) {
	int i;
	size_t len;

	for (;;) {
		if (!fgets(buffer, max_len, fp)) {
			return 0;
		}

		for (len = strlen(buffer), i = 0; 
			(IS_EMPTY_CHAR(buffer[i])); ++i) 
			;

		if (i >= len || buffer[i] == '#') {
			continue;
		}

		return buffer + i;
	}
}

static b32 PLG_LoadMeshData(MeshData *md, FILE *fp, Vec3 world_pos, Vec3 scale, Vec3 rot) {
	char buffer[MAX_PLG_LINE_LEN];
	const char *parsed_string;

	// zero out the mesh
	memset(md, 0, sizeof(MeshData));
	md->state = POLY_STATE_ACTIVE | POLY_STATE_VISIBLE;
	md->world_pos = world_pos;

	// get the object desc
	if (!(parsed_string = PLG_ParseLine(buffer, MAX_PLG_LINE_LEN - 1, fp))) {
		Sys_Print("Error while reading lines from an opened PLG file, it should be a name of the mesh to be loaded,"
				  "number of verts and number polys");
		return false;
	}

	sscanf_s(parsed_string, "%s %d %d", md->name, sizeof(md->name), &md->num_verts, &md->num_polys);

	int num_verts = md->num_verts;
	for (int i = 0; i < num_verts; ++i) {
		if (!(parsed_string = PLG_ParseLine(buffer, MAX_PLG_LINE_LEN - 1, fp))) {
			Sys_Print("Error while reading lines from an opened PLG file, it should be a vertex in the x y z order");
			return false;
		}

		sscanf_s(parsed_string, "%f %f %f",
				 &md->local_vertex_array[i].v.x,
				 &md->local_vertex_array[i].v.y,
				 &md->local_vertex_array[i].v.z);
		//md->local_vertex_list[i].v.w = 1.0f;		disabled the Vec4 for now

		md->local_vertex_array[i].v.x *= scale.v.x;
		md->local_vertex_array[i].v.y *= scale.v.y;
		md->local_vertex_array[i].v.z *= scale.v.z;
	}

	// TODO: compute the avg and max radius for md here

	int poly_num_verts = 0;

	int poly_surface_desc = 0;
	char tmp_poly_surface_desc[8];

	int num_polys = md->num_polys;
	for (int i = 0; i < num_polys; ++i) {
		if (!(parsed_string = PLG_ParseLine(buffer, MAX_PLG_LINE_LEN - 1, fp))) {
			Sys_Print("Error while reading lines from an opened PLG file," 
					  "it should be a 16 bit value in the form of PLG/X format: CSSD | RRRR| GGGG | BBBB");
			return false;
		}

		sscanf_s(parsed_string, "%s %d %d %d %d", 
				 tmp_poly_surface_desc,
				 sizeof(tmp_poly_surface_desc),
				 &poly_num_verts,
				 &md->poly_array[i].vert_indices[0],
				 &md->poly_array[i].vert_indices[1],
				 &md->poly_array[i].vert_indices[2]);

		if (tmp_poly_surface_desc[0] == '0' && tmp_poly_surface_desc[1] == 'x') {
			sscanf_s(tmp_poly_surface_desc, "%x", &poly_surface_desc);
		} else {
			poly_surface_desc = atoi(tmp_poly_surface_desc);
		}

		md->poly_array[i].vertex_list = md->local_vertex_array;

		if (poly_surface_desc & PLX_2SIDED_FLAG) {
			md->poly_array[i].attr |= POLY_ATTR_2SIDED;
		}

		if (poly_surface_desc & PLX_COLOR_MODE_RGB_FLAG) {
			md->poly_array[i].attr |= POLY_ATTR_RGB16;
			int red = (poly_surface_desc & 0x0f00) >> 8;
			int green = (poly_surface_desc & 0x00f0) >> 4;
			int blue = (poly_surface_desc & 0x000f);

			md->poly_array[i].color = RGB_888To565(red * 16, green * 16, blue * 16);
		} else {
			md->poly_array[i].attr |= POLY_ATTR_8BITCOLOR;
			md->poly_array[i].color = poly_surface_desc & 0x00ff;
		}

		int shade_mode = poly_surface_desc & PLX_SHADE_MODE_MASK;

		switch (shade_mode) {
			case PLX_SHADE_MODE_PURE_FLAG: {
				md->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_PURE;
			} break;
			case PLX_SHADE_MODE_FLAT_FLAG: {
				md->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_FLAT;
			} break;
			case PLX_SHADE_MODE_GOURAUD_FLAG: {
				md->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_GOURAUD;
			} break;
			case PLX_SHADE_MODE_PHONG_FLAG: {
				md->poly_array[i].attr |= POLY_ATTR_SHADE_MODE_PHONG;
			} break;
			default: {
				Sys_Print("\nShading mode is invalid, it should be 1 of: 0x0000, 0x2000, 0x4000, 0x6000\n");
			}
		}

		md->poly_array[i].state = POLY_STATE_ACTIVE;
	}

	Sys_Print("\nMesh data loading complete\n");
	return true;
}

void PLG_InitParsing(const char *plg_file_name, MeshData *md, MeshData *player_md) {
	// just for prototyping purposes
	// fixme: replace the CRT file i/o with win32 api
	FILE *fp;
	fopen_s(&fp, plg_file_name, "r");

	if (!fp) {
		Sys_Print("\nCouldn't open file\n");
		Sys_Quit();
	}

	Sys_Print("\nOpening PLG file\n");

	Vec3 world_pos;
	Vector3Init(world_pos, 0.0f, 0.0f, 80.0f);
	Vec3 scale;
	Vector3Init(scale, 1.0f, 1.0f, 1.0f);
	Vec3 rot;
	Vector3Init(rot, 0.0f, 0.0f, 0.0f);

	PLG_LoadMeshData(md, fp, world_pos, scale, rot);

	memcpy(player_md, md, sizeof(MeshData));
	Vector3Init(player_md->world_pos, 400.0f, -50.0f, 400.0f);


	if (fp) {
		Sys_Print("\nClosing PLG file\n");
		fclose(fp);
	}
}