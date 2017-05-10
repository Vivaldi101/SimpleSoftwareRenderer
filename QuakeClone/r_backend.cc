#include "r_cmds.h"
#include "renderer.h"

// Cohen-Sutherland clipping constants
#define INSIDE	 0	
#define LEFT	 1	
#define RIGHT	 2	
#define BOTTOM	 4	
#define TOP		 8  
static int RB_GetLineClipCode(int x, int y, int width, int height) {
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
static void RB_DrawLine(byte *buffer, u32 pitch, int bpp, u32 color, int x0, int y0, int x1, int y1, int width, int height) {
	int outcode0 = RB_GetLineClipCode(x0, y0, width, height);
	int outcode1 = RB_GetLineClipCode(x1, y1, width, height);
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
				outcode0 = RB_GetLineClipCode(x0, y0, width, height);
			} else {
				x1 = (int)x;
				y1 = (int)y;
				outcode1 = RB_GetLineClipCode(x1, y1, width, height);
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
		line = (line + (pitch * y0)) + x0 * bpp;

		for (int i = 0; i < num_pixels; ++i) {
			for (int j = 0; j < bpp; ++j) {
				line[j] = (color >> (j * 8)) & 0xff;
			}
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
static void RB_DrawFlatBottomTriangle(byte *buffer, u32 pitch, int bpp, u32 color, r32 x0, r32 y0, r32 x1, r32 y1, r32 x2, r32 y2, int width, int height) {
	r32 t = x1;
	x1 = (x1 < x2) ? x2 : x1;
	x2 = (t < x2) ? t : x2;

	int cy0 = (int)ceil(y0);
	int cy2 = (int)(ceil(y2) - 1);

	r32 dxy_left = (r32)(x2 - x0) / (r32)(y2 - y0);
	r32 dxy_right = (r32)(x1 - x0) / (r32)(y1 - y0);

	r32 xs = x0;
	r32 xe = x0 + 1.0f;
	xs = xs + ((cy0 - y0) * dxy_left);
	xe = xe + ((cy0 - y0) * dxy_right);
	for (int y = cy0; y <= cy2; ++y) {
		RB_DrawLine(buffer, pitch, bpp, color, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, width, height); 
		xs += dxy_left;
		xe += dxy_right;
	}
}
#endif

// FIXME: pack the points into structures
static void RB_DrawFlatTopTriangle(byte *buffer, u32 pitch, int bpp, u32 color, r32 x0, r32 y0, r32 x1, r32 y1, r32 x2, r32 y2, int width, int height) {
	r32 t = x1;
	x1 = (x1 < x0) ? x0 : x1;
	x0 = (t < x0) ? t : x0;

	int cy0 = (int)ceil(y0);
	int cy2 = (int)(ceil(y2) - 1);

	r32 dxy_left = (r32)(x2 - x0) / (r32)(y2 - y0);
	r32 dxy_right = (r32)(x2 - x1) / (r32)(y2 - y1);

	r32 xs = x0;
	r32 xe = x1 + 1.0f;
	xs = xs + ((cy0 - y0) * dxy_left);
	xe = xe + ((cy0 - y0) * dxy_right);
	for (int y = cy0; y <= cy2; ++y) {
		RB_DrawLine(buffer, pitch, bpp, color, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, width, height); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

static void RB_DrawWireframeMesh(Poly *polys, Vec3 *verts, byte *buffer, int pitch, int bpp, int width, int height, int num_polys) {
	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		Vec3 v0 = polys[i].vertex_array[0];
		Vec3 v1 = polys[i].vertex_array[1];
		Vec3 v2 = polys[i].vertex_array[2];

		RB_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)(v0.v.x + 0.5f),
				   (int)(v0.v.y + 0.5f),
				   (int)(v1.v.x + 0.5f),
				   (int)(v1.v.y + 0.5f),
				   width, height);

		RB_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)(v1.v.x + 0.5f),
				   (int)(v1.v.y + 0.5f),
				   (int)(v2.v.x + 0.5f),
				   (int)(v2.v.y + 0.5f),
				   width, height);

		RB_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)(v2.v.x + 0.5f),
				   (int)(v2.v.y + 0.5f),
				   (int)(v0.v.x + 0.5f),
				   (int)(v0.v.y + 0.5f),
				   width, height);
	}
}

#if 1
static void RB_DrawSolidMesh(Poly *polys, Vec3 *verts, byte *buffer, int pitch, int bpp, int width, int height, int num_polys) {
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

		// sort v0, v1, v2 in ascending y order
		if (y1 < y0) {
			AnySwap(x1, x0, r32);
			AnySwap(y1, y0, r32);
		} 

		// now we know that v0 and v1 are in order 
		if (y2 < y0) {
			AnySwap(x2, x0, r32);
			AnySwap(y2, y0, r32);
		} 

		if (y2 < y1) {
			AnySwap(x2, x1, r32);
			AnySwap(y2, y1, r32);
		} 

		// m = (y - y0) / (x - x0)
		// m(x - x0) = y - y0
		// mx - mx0 = y - y0
		// x - x0 = (y - y0) / m
		// x = (y - y0) / m + x0
		// x derived from the point-slope form of the line
		// split the triangle into 2 parts
		r32 m = (y2 - y0) / (x2 - x0);
		r32 x = (y1 - y0) / m + x0;
		RB_DrawFlatBottomTriangle(buffer, pitch, bpp, polys[i].color, x0, y0, x, y1, x1, y1, width, height);
		RB_DrawFlatTopTriangle(buffer, pitch, bpp, polys[i].color, x1, y1, x, y1, x2, y2, width, height);
	}
}
#endif

#if 0
void RB_DrawRect(byte *buffer, int pitch, int bpp, int width, int height) {
	s32 min_x = roundReal32ToS32(rmin_x);
	s32 min_y = roundReal32ToS32(rmin_y);

	s32 max_x = roundReal32ToS32(rmax_x);
	s32 max_y = roundReal32ToS32(rmax_y);

	u32 pitch	= vs->pitch;
	int bpp		= vs->bpp;

	byte *start = vs->buffer;
	byte *ptr = start + (pitch * min_y) + (min_x * bpp);	

	for (int i = min_y; i < max_y; ++i){
		byte *pixel	= ptr;
		for (int j = min_x; j < max_x; ++j) {
			*pixel++ = color;
		}
		ptr += pitch;
	}
}
#endif

static void RB_ClearMemset(void *buffer, size_t size) {
	memset(buffer, 0, size);
}

static void RB_Blit(HDC hdc, HDC hdc_dib, Vec2i min_xy, Vec2i max_xy) {
	BitBlt(hdc, min_xy[0], min_xy[1], max_xy[0], max_xy[1], hdc_dib, 0, 0, SRCCOPY);
}

static const void *RB_DrawMesh(RenderTarget *rt, const void *data) {
	DrawPolyListCmd *cmd = (DrawPolyListCmd *)data;

	if (cmd->is_wireframe) {
		RB_DrawWireframeMesh(cmd->polys, cmd->poly_verts, rt->buffer, rt->pitch, rt->bpp, rt->width, rt->height, cmd->num_polys);
	} else {
		RB_DrawSolidMesh(cmd->polys, cmd->poly_verts, rt->buffer, rt->pitch, rt->bpp, rt->width, rt->height, cmd->num_polys);
	}

	return (const void *)(cmd + 1);
}

static void RB_DrawFont() {
}

static const void *RB_DrawRect(RenderTarget *rt, const void *data) {
	DrawRectCmd *cmd = (DrawRectCmd *)data;

	if (cmd->type == RECT_FONT) {
		// render fonts
		RB_DrawFont();
	}


	return (const void *)(cmd + 1);
}

static const void *RB_SwapBuffers(RenderTarget *rt, const void *data) {
	SwapBuffersCmd *cmd = (SwapBuffersCmd *)data;
	RB_Blit(cmd->hdc, cmd->hdc_dib_section, MakeVec2i(0, 0), MakeVec2i(rt->width, rt->height));

	return (const void *)(cmd + 1);
}

static const void *RB_ClearBuffer(RenderTarget *rt, const void *data) {
	ClearBufferCmd *cmd = (ClearBufferCmd *)data;
	RB_ClearMemset(rt->buffer, cmd->size);

	return (const void *)(cmd + 1);
}

void RB_ExecuteRenderCommands(RenderTarget *rt, const void *data) {
	for (;;) {
		switch (*(const int *)data) {
			case RCMD_CLEAR:
				data = RB_ClearBuffer(rt, data);
				break;
			case RCMD_SWAP_BUFFERS:
				data = RB_SwapBuffers(rt, data);
				break;
			case RCMD_RECT:
				data = RB_DrawRect(rt, data);
				break;
			case RCMD_MESH:
				data = RB_DrawMesh(rt, data);
				break;
			case RCMD_END_OF_CMDS:
				return;
			default:
				InvalidCodePath("Invalid render command");
		}
	}
}
