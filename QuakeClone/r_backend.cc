#include "r_cmds.h"
#include "renderer.h"

// cohen-sutherland clipping constants
#define INSIDE	 0	
#define LEFT	 1	
#define RIGHT	 2	
#define BOTTOM	 4	
#define TOP		 8  
static int R_GetLineClipCode(int x, int y, int width, int height) {
	int code = INSIDE;												

	if (x < 0) {													
		code |= LEFT;
	} else if (x >= width) {			
		code |= RIGHT;
	}
	if (y < 0) {													
		code |= BOTTOM;
	} else if (y >= height) {		
		code |= TOP;
	}

	return code;
}

// FIXME: pack the points into structures
static void R_DrawLine(byte *buffer, u32 pitch, u32 bpp, u32 color, int x0, int y0, int x1, int y1, int width, int height) {
	int outcode0 = R_GetLineClipCode(x0, y0, width, height);
	int outcode1 = R_GetLineClipCode(x1, y1, width, height);
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
				y = (r32)height - 1.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & BOTTOM) { // point is below the clip rectangle
				y = 0.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & RIGHT) {  // point is to the right of clip rectangle
				x = (r32)width - 1.0f;
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
				outcode0 = R_GetLineClipCode(x0, y0, width, height);
			} else {
				x1 = (int)x;
				y1 = (int)y;
				outcode1 = R_GetLineClipCode(x1, y1, width, height);
			}
		}
	}
	if (accept) {
		// Bresenham
		// FIXME: improve on this
		int dx			= abs(x1 - x0);
		int dy			= abs(y1 - y0);
		int stride		= pitch / bpp;
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
			y1_inc = stride;
			y2_inc = stride;
		} else {		// bottom to top
			y1_inc = -stride;
			y2_inc = -stride;
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
		u32 *line = (u32*)buffer;
		line = (line + (stride * y0)) + x0;

		for (int i = 0; i < num_pixels; ++i) {
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

#if 0
// FIXME: pack the points into structures
static void R_DrawFlatBottomTriangle(VidSystem *vs, r32 p0_x, r32 p0_y, r32 p1_x, r32 p1_y, r32 p2_x, r32 p2_y, Poly *poly) {
	if (p1_x < p2_x) {
		AnySwap(p1_x, p2_x, r32);
	}

	int cp0_y = (int)ceil(p0_y);
	int cp2_y = (int)(ceil(p2_y) - 1);

	r32 dxy_left = (r32)(p2_x - p0_x) / (r32)(p2_y - p0_y);
	r32 dxy_right = (r32)(p1_x - p0_x) / (r32)(p1_y - p0_y);

	r32 xs = p0_x;
	r32 xe = p0_x;
	xs = xs + ((cp0_y - p0_y) * dxy_left);
	xe = xe + ((cp0_y - p0_y) * dxy_right);
	for (int y = cp0_y; y <= cp2_y; ++y) {
		R_DrawLine(vs, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, poly->color); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

// FIXME: pack the points into structures
static void R_DrawFlatTopTriangle(VidSystem *vs, r32 p0_x, r32 p0_y, r32 p1_x, r32 p1_y, r32 p2_x, r32 p2_y, Poly *poly) {
	if (p1_x < p0_x) {
		AnySwap(p1_x, p0_x, r32);
	}

	int cp0_y = (int)ceil(p0_y);
	int cp2_y = (int)(ceil(p2_y) - 1);

	r32 dxy_left = (r32)(p2_x - p0_x) / (r32)(p2_y - p0_y);
	r32 dxy_right = (r32)(p2_x - p1_x) / (r32)(p2_y - p1_y);

	r32 xs = p0_x;
	r32 xe = p1_x;
	xs = xs + ((cp0_y - p0_y) * dxy_left);
	xe = xe + ((cp0_y - p0_y) * dxy_right);
	for (int y = cp0_y; y <= cp2_y; ++y) {
		R_DrawLine(vs, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, poly->color); 
		xs += dxy_left;
		xe += dxy_right;
	}
}
#endif

void R_DrawWireframeMesh(Poly *polys, Vec3 *verts, byte *buffer, u32 pitch, u32 bpp, int width, int height, int num_polys) {
	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		R_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)verts[v0].v.x,
				   (int)verts[v0].v.y,
				   (int)verts[v1].v.x,
				   (int)verts[v1].v.y,
				   width, height);

		R_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)verts[v1].v.x,
				   (int)verts[v1].v.y,
				   (int)verts[v2].v.x,
				   (int)verts[v2].v.y,
				   width, height);

		//R_DrawLine((int)verts[v2].v.x,
		//		   (int)verts[v2].v.y,
		//		   (int)verts[v0].v.x,
		//		   (int)verts[v0].v.y,
		//		   polys[i].color);

		R_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)verts[v2].v.x,
				   (int)verts[v2].v.y,
				   (int)verts[v0].v.x,
				   (int)verts[v0].v.y,
				   width, height);
	}
}

#if 0
void R_DrawSolidMesh(Poly *polys, Vec3 *verts, int num_polys) {
	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		r32 p0_x = verts[v0].v.x;
		r32 p0_y = verts[v0].v.y;

		r32 p1_x = verts[v1].v.x;
		r32 p1_y = verts[v1].v.y;

		r32 p2_x = verts[v2].v.x;
		r32 p2_y = verts[v2].v.y;

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
			R_DrawFlatTopTriangle(vs, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, &polys[i]);
		} else if (p1_y == p2_y) {
			// flat bottom
			R_DrawFlatBottomTriangle(vs, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, &polys[i]);
		} else {
			// split the triangle into 2 parts

			// m = (y - y0) / (x - x0)
			// m(x - x0) = y - y0
			// mx - mx0 = y - y0
			// x - x0 = (y - y0) / m
			// x = (y - y0) / m + x0
			// x derived from the point-slope form of the line
			r32 m = (p2_y - p0_y) / (p2_x - p0_x);
			r32 x = (p1_y - p0_y) / m + p0_x;
			R_DrawFlatBottomTriangle(vs, p0_x, p0_y, x, p1_y, p1_x, p1_y, &polys[i]);
			R_DrawFlatTopTriangle(vs, p1_x, p1_y, x, p1_y, p2_x, p2_y, &polys[i]);
		}
	}
}
#endif

static void R_DrawRect(byte *buffer, u32 pitch, 
				       u32 bpp, int width, int height,
				       r32 r, r32 g, r32 b) {
	Assert(bpp == 4);
	u32 color = (roundReal32ToU32(r * 255.0f) << 16 |
				 roundReal32ToU32(g * 255.0f) << 8  |
				 roundReal32ToU32(b * 255.0f));
		
	byte *ptr = buffer;

	for (int i = 0; i < height; ++i){
		u32 *pixel	= (u32*)ptr;
		for (int j = 0; j < width; ++j) {
			*pixel++ = color;
		}
		ptr += pitch;
	}
}

void R_DrawRect(VidSystem *vs, r32 rmin_x, r32 rmin_y, 
				r32 rmax_x, r32 rmax_y,
				r32 r, r32 g, r32 b) {
	i32 min_x = roundReal32ToI32(rmin_x);
	i32 min_y = roundReal32ToI32(rmin_y);

	i32 max_x = roundReal32ToI32(rmax_x);
	i32 max_y = roundReal32ToI32(rmax_y);

	if (min_x < 0) {
		min_x = 0;
	}
	if (min_y < 0) {
		min_y = 0;
	}
	if (max_x > vs->width) {
		max_x = vs->width;
	}
	if (max_y > vs->height) {
		max_y = vs->height;
	}

	u32 pitch	= vs->pitch;
	u32 bpp		= vs->bpp;

	Assert(bpp == 4);
	u32 color	= (roundReal32ToU32(r * 255.0f) << 16 |
				   roundReal32ToU32(g * 255.0f) << 8  |
				   roundReal32ToU32(b * 255.0f));
		
	byte *start = vs->buffer;
	byte *ptr = start + (pitch * min_y) + (min_x * bpp);	

	for (int i = min_y; i < max_y; ++i){
		u32 *pixel	= (u32*)ptr;
		for (int j = min_x; j < max_x; ++j) {
			*pixel++ = color;
		}
		ptr += pitch;
	}
}

static void RB_ClearMemset(void *buffer, u32 pitch, int height) {
	memset(buffer, 40, pitch * height);
}

static void RB_Blit(HDC hdc, HDC hdc_dib, int x, int y, int width, int height) {
	BitBlt(hdc, x, y, width, height, hdc_dib, 0, 0, SRCCOPY);
}

static const void *RB_DrawMesh(const void *data) {
	DrawMeshCmd *cmd = (DrawMeshCmd *)data;

	if (cmd->solid) {
		//R_DrawSolidMesh();
	} else {
		R_DrawWireframeMesh(cmd->polys, cmd->verts, cmd->buffer, cmd->pitch, cmd->bpp, cmd->width, cmd->height, cmd->num_polys);
	}

	return (const void *)(cmd + 1);
}

static const void *RB_SwapBuffers(const void *data) {
	SwapBuffersCmd *cmd = (SwapBuffersCmd *)data;
	RB_Blit(cmd->hdc, cmd->hdc_dib_section, 0, 0, cmd->width, cmd->height);

	return (const void *)(cmd + 1);
}

static const void *RB_ClearBuffer(const void *data) {
	ClearBufferCmd *cmd = (ClearBufferCmd *)data;
	RB_ClearMemset(cmd->buffer, cmd->pitch, cmd->height);

	return (const void *)(cmd + 1);
}

void RB_ExecuteRenderCommands(const void *data) {
	for (;;) {
		switch (*(const int *)data) {
			case RCMD_CLEAR:
				data = RB_ClearBuffer(data);
				break;
			case RCMD_SWAP_BUFFERS:
				data = RB_SwapBuffers(data);
				break;
			case RCMD_MESH:
				data = RB_DrawMesh(data);
				break;
			case RCMD_END_OF_CMDS:
				return;
			default:
				InvalidCodePath("Invalid render command");
		}
	}
}
