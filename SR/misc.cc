#include "shared.h"

#if 0
// print the palette
	int c = 0;
	for (int i = 1; i <= 16; ++i) {
		for (int j = 1; j <= 16; ++j) {
			R_DrawRect(rbe->rt, j * 20.0f, i * 20.0f, j * 20.0f + 20.0f, i * 20.0f + 20.0f, c);
			++c;
		}
	}

// print the colormap
void draw_bmp(byte *buffer, int w, int h) {
	BitBlt(rt->win_handles.hdc, 0, 0, w, h, rt->win_handles.hdc_dib_section, 0, 0, SRCCOPY);
}
#endif


#if 0

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
      }
}
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

		// FIXME: stop passing bpp and pitch or make them cvars?
		Assert(bpp == 4);
		byte *line = (byte*)buffer;
		line = (line + (pitch * y0)) + x0 * bpp;

		for (int i = 0; i < num_pixels; i++) {
			//for (int j = 0; j < bpp; ++j) {
			u32 *pixel = (u32 *)line;
			*pixel = color;
			//}
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
	for (int y = cy0; y <= cy2; y++) {
		RB_DrawLine(buffer, pitch, bpp, color, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, width, height); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

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
	for (int y = cy0; y <= cy2; y++) {
		RB_DrawLine(buffer, pitch, bpp, color, (int)(xs + 0.5f), y, (int)(xe + 0.5f), y, width, height); 
		xs += dxy_left;
		xe += dxy_right;
	}
}

// FIXME: change width and height to render_target_*
static void RB_DrawWireframeMesh(Poly *polys, PolyVert *poly_verts, byte *buffer, int pitch, int bpp, int width, int height, int num_polys) {
	for (int i = 0; i < num_polys; i++) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

		PolyVert v0 = polys[i].vertex_array[0];
		PolyVert v1 = polys[i].vertex_array[1];
		PolyVert v2 = polys[i].vertex_array[2];

		RB_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)(v0.xyz.v.x + 0.5f),
				   (int)(v0.xyz.v.y + 0.5f),
				   (int)(v1.xyz.v.x + 0.5f),
				   (int)(v1.xyz.v.y + 0.5f),
				   width, height);

		RB_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)(v1.xyz.v.x + 0.5f),
				   (int)(v1.xyz.v.y + 0.5f),
				   (int)(v2.xyz.v.x + 0.5f),
				   (int)(v2.xyz.v.y + 0.5f),
				   width, height);

		RB_DrawLine(buffer, pitch, bpp, polys[i].color,
				   (int)(v2.xyz.v.x + 0.5f),
				   (int)(v2.xyz.v.y + 0.5f),
				   (int)(v0.xyz.v.x + 0.5f),
				   (int)(v0.xyz.v.y + 0.5f),
				   width, height);
	}
}
#endif
#if 0
static void RB_DrawFilledRect(RenderTarget *rt, Bitmap *bm, Vec2 origin, Vec2 x, Vec2 y) {
	Vec2 points[4] = {origin, origin + x, origin + y, origin + x + y};
	s32 min_x = SINT32_MAX, min_y = SINT32_MAX, max_x = SINT32_MIN, max_y = SINT32_MIN;
	int pitch = rt->pitch;
	byte *src = bm->data, *dst = 0;
	u32 *src_pixel = (u32 *)src;

	for (int i = 0; i < 4; ++i) {
		s32 floor_x = (s32)floor(points[i][0]);
		s32 floor_y = (s32)floor(points[i][1]);
		s32 ceil_x = (s32)ceil(points[i][0]);
		s32 ceil_y = (s32)ceil(points[i][1]);

		min_x = (min_x > floor_x) ? floor_x : min_x;
		min_y = (min_y > floor_y) ? floor_y : min_y;

		max_x = (max_x < ceil_x) ? ceil_x : max_x;
		max_y = (max_y < ceil_y) ? ceil_y : max_y;
	}
	Assert(min_x >= 0 && min_y >= 0 && max_x <= rt->width && max_y <= rt->height);
	dst = rt->buffer + (pitch * min_y) + (min_x * rt->bpp);	

	for (int i = min_y; i < max_y; ++i){
		u32 *dst_pixel = (u32 *)dst;
		for (int j = min_x; j < max_x; ++j) {
			Vec2 p = {(r32)j, (r32)i};
			r32 edge0 = Dot2(-Perp(x), p - origin);
			r32 edge1 = Dot2(Perp(y), p - (origin + y));
			r32 edge2 = Dot2(Perp(x), p - (origin + x + y));
			r32 edge3 = Dot2(-Perp(y), p - (origin + x));

			if (edge0 <= 0.0f && edge1 <= 0.0f && edge2 <= 0.0f && edge3 <= 0.0f) {
				*dst_pixel = 255 << 8;
			} else {
				//*dst_pixel = 255 << 0;
			}
			++dst_pixel;
		}
		dst += pitch;
	}
}


static const void *RB_DrawRect(RenderTarget *rt, const void *data) {
	DrawRectCmd *cmd = (DrawRectCmd *)data;
	//RB_DrawFilledRect(rt, &cmd->bitmap, cmd->basis.origin, cmd->basis.axis[0], cmd->basis.axis[1]);

	return (const void *)(cmd + 1);
}
#endif
