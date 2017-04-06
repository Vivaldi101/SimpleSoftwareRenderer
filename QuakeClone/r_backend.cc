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
static void R_DrawLine(byte *buffer, u32 pitch, int bpp, u32 color, int x0, int y0, int x1, int y1, int width, int height) {
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
			r32 x = 0.0f;
			r32 y = 0.0f;
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
		byte *line = (byte*)buffer;
		line = (line + (stride * (y0 * bpp))) + x0 * bpp;

		for (int i = 0; i < num_pixels; ++i) {
			*line = 100;
			numerator += add;
			if (numerator >= denominator) {
				numerator -= denominator;
				line += (x2_inc * bpp);	// inc x in case of y-dominant line
				line += (y2_inc * bpp); // inc y in case of x-dominant line
			}
			line += (x1_inc * bpp);
			line += (y1_inc * bpp);
		}
	}
}

#if 1
// FIXME: pack the points into structures
static void R_DrawFlatBottomTriangle(byte *buffer, u32 pitch, int bpp, u32 color, r32 x0, r32 y0, r32 x1, r32 y1, r32 x2, r32 y2, int width, int height) {
	if (x1 < x2) {
		AnySwap(x1, x2, r32);
	}

	int cy0 = (int)ceil(y0);
	int cy2 = (int)(ceil(y2) - 1);

	r32 dxy_left = (r32)(x2 - x0) / (r32)(y2 - y0);
	r32 dxy_right = (r32)(x1 - x0) / (r32)(y1 - y0);

	r32 xs = x0;
	r32 xe = x0;
	xs = xs + ((cy0 - y0) * dxy_left);
	xe = xe + ((cy0 - y0) * dxy_right);
	for (int y = cy0; y <= cy2; ++y) {
		R_DrawLine(buffer, pitch, bpp, color, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, width, height); 
		xs += dxy_left;
		xe += dxy_right;
	}
}
#endif

// FIXME: pack the points into structures
static void R_DrawFlatTopTriangle(byte *buffer, u32 pitch, int bpp, u32 color, r32 x0, r32 y0, r32 x1, r32 y1, r32 x2, r32 y2, int width, int height) {
	if (x1 < x0) {
		AnySwap(x1, x0, r32);
	}

	int cy0 = (int)ceil(y0);
	int cy2 = (int)(ceil(y2) - 1);

	r32 dxy_left = (r32)(x2 - x0) / (r32)(y2 - y0);
	r32 dxy_right = (r32)(x2 - x1) / (r32)(y2 - y1);

	r32 xs = x0;
	r32 xe = x1;
	xs = xs + ((cy0 - y0) * dxy_left);
	xe = xe + ((cy0 - y0) * dxy_right);
	for (int y = cy0; y <= cy2; ++y) {
		R_DrawLine(buffer, pitch, bpp, color, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, width, height); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

void R_DrawWireframeMesh(Poly *polys, Vec3 *verts, byte *buffer, u32 pitch, int bpp, u32 color, int width, int height, int num_polys) {
	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		Vec3 v0 = polys[i].vertex_array[0];
		Vec3 v1 = polys[i].vertex_array[1];
		Vec3 v2 = polys[i].vertex_array[2];

		R_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)v0.v.x,
				   (int)v0.v.y,
				   (int)v1.v.x,
				   (int)v1.v.y,
				   width, height);

		R_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)v1.v.x,
				   (int)v1.v.y,
				   (int)v2.v.x,
				   (int)v2.v.y,
				   width, height);

		R_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)v2.v.x,
				   (int)v2.v.y,
				   (int)v0.v.x,
				   (int)v0.v.y,
				   width, height);
	}
}

#if 1
void R_DrawSolidMesh(Poly *polys, Vec3 *verts, byte *buffer, u32 pitch, int bpp, u32 color, int width, int height, int num_polys) {
	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		Vec3 v0 = polys[i].vertex_array[0];
		Vec3 v1 = polys[i].vertex_array[1];
		Vec3 v2 = polys[i].vertex_array[2];

		r32 x0 = v0.v.x;
		r32 y0 = v0.v.y;

		r32 x1 = v1.v.x;
		r32 y1 = v1.v.y;

		r32 x2 = v2.v.x;
		r32 y2 = v2.v.y;

		// sort p0, p1, p2 in ascending y order
		if (y1 < y0) {
			AnySwap(x1, x0, r32);
			AnySwap(y1, y0, r32);
		} 

		// now we know that p0 and p1 are in order 
		if (y2 < y0) {
			AnySwap(x2, x0, r32);
			AnySwap(y2, y0, r32);
		} 

		if (y2 < y1) {
			AnySwap(x2, x1, r32);
			AnySwap(y2, y1, r32);
		} 

		if (y0 == y1) {
			// flat top
			R_DrawFlatTopTriangle(buffer, pitch, bpp, color, x0, y0, x1, y1, x2, y2, width, height);
		} else if (y1 == y2) {
			// flat bottom
			R_DrawFlatBottomTriangle(buffer, pitch, bpp, color, x0, y0, x1, y1, x2, y2, width, height);
		} else {
			// split the triangle into 2 parts

			// m = (y - y0) / (x - x0)
			// m(x - x0) = y - y0
			// mx - mx0 = y - y0
			// x - x0 = (y - y0) / m
			// x = (y - y0) / m + x0
			// x derived from the point-slope form of the line
			r32 m = (y2 - y0) / (x2 - x0);
			r32 x = (y1 - y0) / m + x0;
			R_DrawFlatBottomTriangle(buffer, pitch, bpp, color, x0, y0, x, y1, x1, y1, width, height);
			R_DrawFlatTopTriangle(buffer, pitch, bpp, color, x1, y1, x, y1, x2, y2, width, height);
		}
	}
}
#endif

static void R_DrawRect(byte *buffer, u32 pitch, 
				       int bpp, int width, int height,
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
	int bpp		= vs->bpp;

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

static void RB_Blit(HDC hdc, HDC hdc_dib, Vec2 min_xy, Vec2 max_xy) {
	BitBlt(hdc, (int)min_xy[0], (int)min_xy[1], (int)max_xy[0], (int)max_xy[1], hdc_dib, 0, 0, SRCCOPY);
}

static const void *RB_DrawMesh(const void *data) {
	DrawPolyListCmd *cmd = (DrawPolyListCmd *)data;

	if (cmd->is_wireframe) {
		R_DrawWireframeMesh(cmd->polys, cmd->poly_verts, cmd->buffer, cmd->pitch, cmd->bpp, cmd->color, cmd->width, cmd->height, cmd->num_polys);
	} else {
		R_DrawSolidMesh(cmd->polys, cmd->poly_verts, cmd->buffer, cmd->pitch, cmd->bpp, cmd->color, cmd->width, cmd->height, cmd->num_polys);
	}

	return (const void *)(cmd + 1);
}

static const void *RB_SwapBuffers(const void *data) {
	SwapBuffersCmd *cmd = (SwapBuffersCmd *)data;
	RB_Blit(cmd->hdc, cmd->hdc_dib_section, MakeVec2(0.0f, 0.0f), MakeVec2((r32)cmd->width, (r32)cmd->height));

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
