#include "shared.h"
#include "renderer.h"

void R_BeginFrame(Renderer *ren, byte fill_color) {
	// clean up the framebuffer
	void *buffer = ren->vid_sys.buffer;
	int pitch = ren->vid_sys.pitch;
	int height = ren->vid_sys.height;
	memset(buffer, fill_color, pitch * height);
}

void R_EndFrame(Renderer *ren) {
	if (!ren->vid_sys.state.full_screen) {
		BitBlt(ren->vid_sys.win_handles.dc,
			   0, 0,
			   ren->vid_sys.width,
			   ren->vid_sys.height,
			   ren->vid_sys.win_handles.hdc_dib_section,
			   0, 0, SRCCOPY);
	}
}

// cohen-sutherland clipping constants
#define INSIDE	 0	
#define LEFT	 1	
#define RIGHT	 2	
#define BOTTOM	 4	
#define TOP		 8  
static int R_GetLineClipCode(Renderer *ren, int x, int y) {
	int code = INSIDE;												// initialised as being inside of [[clip window]]

	if (x < 0) {													// to the left of clip window
		code |= LEFT;
	} else if (x >= ren->vid_sys.width) {			// to the right of clip window
		code |= RIGHT;
	}
	if (y < 0) {													// below the clip window
		code |= BOTTOM;
	} else if (y >= ren->vid_sys.height) {			// above the clip window
		code |= TOP;
	}

	return code;
}

static void R_DrawLine(Renderer *ren, int x0, int y0, int x1, int y1, u32 color) {
	int outcode0 = R_GetLineClipCode(ren, x0, y0);
	int outcode1 = R_GetLineClipCode(ren, x1, y1);
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
				y = (r32)ren->vid_sys.height - 1.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & BOTTOM) { // point is below the clip rectangle
				y = 0.0f;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & RIGHT) {  // point is to the right of clip rectangle
				x = (r32)ren->vid_sys.width - 1.0f;
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
				outcode0 = R_GetLineClipCode(ren, x0, y0);
			} else {
				x1 = (int)x;
				y1 = (int)y;
				outcode1 = R_GetLineClipCode(ren, x1, y1);
			}
		}
	}
	if (accept) {
		// Bresenham
		// FIXME: improve on this
		int dx			= abs(x1 - x0);
		int dy			= abs(y1 - y0);
		int pitch		= ren->vid_sys.pitch / ren->vid_sys.bpp;
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
			y1_inc = pitch;
			y2_inc = pitch;
		} else {		// bottom to top
			y1_inc = -pitch;
			y2_inc = -pitch;
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
		u32 *line = (u32*)ren->vid_sys.buffer;
		line = (line + (pitch * y0)) + x0;

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

static void R_DrawFlatBottomTriangle(Renderer *ren, r32 p0_x, r32 p0_y, r32 p1_x, r32 p1_y, r32 p2_x, r32 p2_y, Poly *poly) {
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
		R_DrawLine(ren, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, poly->color); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

static void R_DrawFlatTopTriangle(Renderer *ren, r32 p0_x, r32 p0_y, r32 p1_x, r32 p1_y, r32 p2_x, r32 p2_y, Poly *poly) {
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
		R_DrawLine(ren, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, poly->color); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

void R_DrawWireframeMesh(Renderer *ren, Entity *md) {
	int num_polys = md->status.num_polys;
	Poly *polys = md->mesh->polys->poly_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		R_DrawLine(ren,
				   (int)trans_verts[v0].v.x,
				   (int)trans_verts[v0].v.y,
				   (int)trans_verts[v1].v.x,
				   (int)trans_verts[v1].v.y,
				   polys[i].color);

		R_DrawLine(ren,
				   (int)trans_verts[v1].v.x,
				   (int)trans_verts[v1].v.y,
				   (int)trans_verts[v2].v.x,
				   (int)trans_verts[v2].v.y,
				   polys[i].color);

		R_DrawLine(ren,
				   (int)trans_verts[v2].v.x,
				   (int)trans_verts[v2].v.y,
				   (int)trans_verts[v0].v.x,
				   (int)trans_verts[v0].v.y,
				   polys[i].color);
	}
}

void R_DrawSolidMesh(Renderer *ren, Entity *md) {
	int num_polys = md->status.num_polys;
	Poly *polys = md->mesh->polys->poly_array;
	Vec3 *trans_verts = md->mesh->trans_verts->vert_array;

	for (int i = 0; i < num_polys; ++i) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		int v0 = polys[i].vert_indices[0];
		int v1 = polys[i].vert_indices[1];
		int v2 = polys[i].vert_indices[2];

		r32 p0_x = trans_verts[v0].v.x;
		r32 p0_y = trans_verts[v0].v.y;

		r32 p1_x = trans_verts[v1].v.x;
		r32 p1_y = trans_verts[v1].v.y;

		r32 p2_x = trans_verts[v2].v.x;
		r32 p2_y = trans_verts[v2].v.y;

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
			R_DrawFlatTopTriangle(ren, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, &polys[i]);
		} else if (p1_y == p2_y) {
			// flat bottom
			R_DrawFlatBottomTriangle(ren, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, &polys[i]);
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
			R_DrawFlatBottomTriangle(ren, p0_x, p0_y, x, p1_y, p1_x, p1_y, &polys[i]);
			R_DrawFlatTopTriangle(ren, p1_x, p1_y, x, p1_y, p2_x, p2_y, &polys[i]);
		}
	}
}
