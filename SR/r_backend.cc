#include "r_cmds.h"
#include "renderer.h"

#define SPS SUB_PIXEL_STEP
#define SUB_PIXEL_POW2 (1<<SUB_PIXEL_STEP)
#define SUB_PIXEL_POW2_MINUS_1 (1<<(SUB_PIXEL_STEP-1))
#define SUB_PIXEL_OFFSET ((1<<SUB_PIXEL_STEP) >> 1)
#define SPS_OFFSET SUB_PIXEL_OFFSET
#define Lerp(s, d, t) ((1.0f-(t))*(s) + (t)*(d))
#define ONE_OVER_SPS_POW2 (1.0f / SUB_PIXEL_POW2)
//#define BILINEAR_FILTER 0

static inline Vec4 Modulate(Vec4 a, Vec4 b) {
	Vec4 result;

	result[0] = a[0]*b[0];
	result[1] = a[1]*b[1];
	result[2] = a[2]*b[2];
	result[3] = a[3]*b[3];

	return result;
}

#if 0
struct ClipEdge {
	s32 x0, y0;
	s32 x1, y1;
};
// Cohen-Sutherland clipping constants
#define INSIDE	 0	
#define LEFT	 1	
#define RIGHT	 2	
#define BOTTOM	 4	
#define TOP		 8  
static int RB_FindClipCode(r32 px, r32 py, s32 xmin, s32 ymin, s32 xmax, s32 ymax) {
	s32 code = INSIDE;												

	if (px < xmin) {													
		code |= LEFT;
	} else if (px > xmax) {			
		code |= RIGHT;
	}
	if (py < ymin) {													
		code |= BOTTOM;
	} else if (py > ymax) {		
		code |= TOP;
	}

	return code;
}

static ClipEdge RB_ClipTestEdge(r32 x0, r32 y0, r32 x1, r32 y1, s32 xmin, s32 ymin, s32 xmax, s32 ymax) {
	ClipEdge ce = {};
	int outcode0 = RB_FindClipCode(x0, y0, xmin, ymin, xmax, ymax);
	int outcode1 = RB_FindClipCode(x1, y1, xmin, ymin, xmax, ymax);
	b32 accept = false;

	for (;;) {
		if (!(outcode0 | outcode1)) {		// trivially accept 
			accept = true;
			break;
		} else if (outcode0 & outcode1) {	// trivially reject 
			break;
		} else {
			r32 x = 0.0f;
			r32 y = 0.0f;
			r32 m = (r32)(y1 - y0) / (r32)(x1 - x0);

			// pick the one that is outside of the clipping rect
			int chosen_code = outcode0 ? outcode0 : outcode1;

			if (chosen_code & TOP) {           
				y = (r32)ymax;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & BOTTOM) { 
				y = (r32)ymin;
				x = (1.0f / m) * (y - y0) + x0;
			} else if (chosen_code & RIGHT) {  
				x = (r32)xmax;
				y = m * (x - x0) + y0;
			} else if (chosen_code & LEFT) {   
				x = (r32)xmin;
				y = m * (x - x0) + y0;
			}

			// now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (chosen_code == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = RB_FindClipCode(x0, y0, xmin, ymin, xmax, ymax);
			} else {
				x1 = x;
				y1 = y;
				outcode1 = RB_FindClipCode(x1, y1, xmin, ymin, xmax, ymax);
			}
		}
	}
	if (accept) {
		ce.x0 = RoundReal32ToS32(x0*ONE_OVER_SPS_POW2);
		ce.y0 = RoundReal32ToS32(y0*ONE_OVER_SPS_POW2);
		ce.x1 = RoundReal32ToS32(x1*ONE_OVER_SPS_POW2);
		ce.y1 = RoundReal32ToS32(y1*ONE_OVER_SPS_POW2);
	} 

	return ce;
}
#endif

static s32 PerpDot(Vec2i a, Vec2i b)
{
	Vec2i perp;

	perp.v.x = -a.v.y;
	perp.v.y = a.v.x;

	return (s32)((Dot2(perp, b) >> SPS));
}

// ccw vertex winding order
static inline b32 IsTopLeftEdge(Point2D a, Point2D b) {
	if (a.y == b.y && a.x > b.x) {	
		return true;	// top
	} else if (a.y > b.y) {			
		return true;	// left
	}
	return false;
}

// FIXME: change width and height to render_target_*
static void RB_DrawSolidMesh(Poly *polys, byte *frame_buffer, r32 depth_buffer[DEPTH_BUFFER_DIM][DEPTH_BUFFER_DIM], Bitmap *texture, int pitch, int bpp, int screen_width, int screen_height, int num_polys) {
	for (int i = 0; i < num_polys; i++) {
		Vec4 poly_color = polys[i].color;

		// original vertices
		PolyVert v0 = polys[i].vertex_array[0];
		PolyVert v1 = polys[i].vertex_array[1];
		PolyVert v2 = polys[i].vertex_array[2];

		Vec2 uv_v0;
		Vec2 uv_v1;
		Vec2 uv_v2;

		uv_v0[0] = v0.uv[0];
		uv_v0[1] = v0.uv[1];

		uv_v1[0] = v1.uv[0];
		uv_v1[1] = v1.uv[1];

		uv_v2[0] = v2.uv[0];
		uv_v2[1] = v2.uv[1];

		s32 tex_width = texture->dim[0];
		s32 tex_height = texture->dim[1];

		// ccw vertex winding order
		s32 x0 = RoundReal32ToS32((v0.xyz.v.x) * SUB_PIXEL_POW2);
		s32 y0 = RoundReal32ToS32((v0.xyz.v.y) * SUB_PIXEL_POW2);
		r32 z0 = v0.xyz.v.z;
		r32 w0 = v0.w;

		s32 x1 = RoundReal32ToS32((v1.xyz.v.x) * SUB_PIXEL_POW2);
		s32 y1 = RoundReal32ToS32((v1.xyz.v.y) * SUB_PIXEL_POW2);
		r32 z1 = v1.xyz.v.z;
		r32 w1 = v1.w;

		s32 x2 = RoundReal32ToS32((v2.xyz.v.x) * SUB_PIXEL_POW2);
		s32 y2 = RoundReal32ToS32((v2.xyz.v.y) * SUB_PIXEL_POW2);
		r32 z2 = v2.xyz.v.z;
		r32 w2 = v2.w;

		// Guard band

		Assert((GUARD_BAND_MIN <= x0) && (x0 <= GUARD_BAND_MAX));
		Assert((GUARD_BAND_MIN <= y0) && (y0 <= GUARD_BAND_MAX));

		Assert((GUARD_BAND_MIN <= x1) && (x1 <= GUARD_BAND_MAX));
		Assert((GUARD_BAND_MIN <= y1) && (y1 <= GUARD_BAND_MAX));

		Assert((GUARD_BAND_MIN <= x2) && (x2 <= GUARD_BAND_MAX));
		Assert((GUARD_BAND_MIN <= y2) && (y2 <= GUARD_BAND_MAX));

		// compute triangle 2d AABB for the scaled points
		s32 min_x = (MIN3(x0, x1, x2));
		s32 min_y = (MIN3(y0, y1, y2));

		s32 max_x = (MAX3(x0, x1, x2));
		s32 max_y = (MAX3(y0, y1, y2));

		Point2D min_pt = {(min_x >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP, (min_y >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP};
		min_pt.x = min_pt.x - SUB_PIXEL_OFFSET;
		min_pt.y = min_pt.y - SUB_PIXEL_OFFSET;
		Point2D max_pt = {(max_x >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP, (max_y >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP};
		max_pt.x = max_pt.x + SUB_PIXEL_OFFSET; 
		max_pt.y = max_pt.y + SUB_PIXEL_OFFSET;

		// clip against the screen
		min_pt.x = MAX(min_pt.x, ((-screen_width/2)<<SPS) + SPS_OFFSET);
		min_pt.y = MAX(min_pt.y, ((-screen_height/2)<<SPS) + SPS_OFFSET);

		max_pt.x = MIN(max_pt.x, ((((screen_width-1)/2)<<SPS) - SPS_OFFSET));
		max_pt.y = MIN(max_pt.y, ((((screen_height-1)/2)<<SPS) - SPS_OFFSET));

		// make separate point structures for edge testing
		Point2D pv0 = {x0, y0};
		Point2D pv1 = {x1, y1};
		Point2D pv2 = {x2, y2};

		s32 tri2d_area = PerpDot(MakeVectorFromPoints(pv0, pv1), MakeVectorFromPoints(pv0, pv2));
		if (tri2d_area <= 0) {                          // degenerate or back-facing triangle
			continue;
		}

		// compute biases for top-left fill convention
		s32 bias0 = IsTopLeftEdge(pv1, pv2) ? 0 : -1;
		s32 bias1 = IsTopLeftEdge(pv2, pv0) ? 0 : -1;
		s32 bias2 = IsTopLeftEdge(pv0, pv1) ? 0 : -1;

		Vec2i v01 = MakeVectorFromPoints(pv0, pv1);
		Vec2i v12 = MakeVectorFromPoints(pv1, pv2);
		Vec2i v20 = MakeVectorFromPoints(pv2, pv0);

		s32 w0_row = PerpDot(v12, MakeVectorFromPoints(pv1, min_pt)) + bias0;
		s32 w1_row = PerpDot(v20, MakeVectorFromPoints(pv2, min_pt)) + bias1;
		s32 w2_row = PerpDot(v01, MakeVectorFromPoints(pv0, min_pt)) + bias2;

		r32 one_over_tri2d_area = 1.0f / (r32)tri2d_area;

		r32 dz10 = z1-z0;
		r32 dz20 = z2-z0;

		// point to test against the triangle
		Point2D pt;

		min_pt.x = min_pt.x; 
		min_pt.y = min_pt.y; 
		max_pt.x = max_pt.x; 
		max_pt.y = max_pt.y; 

		min_pt.x += ((screen_width>>1) << SPS);
		min_pt.y += ((screen_height>>1) << SPS);
		max_pt.x += ((screen_width>>1) << SPS);
		max_pt.y += ((screen_height>>1) << SPS);

		uv_v0[0] /= w0;
		uv_v0[1] /= w0;

		uv_v1[0] /= w1;
		uv_v1[1] /= w1;

		uv_v2[0] /= w2;
		uv_v2[1] /= w2;

		w0 = 1.0f / w0;
		w1 = 1.0f / w1;
		w2 = 1.0f / w2;

		byte *line = frame_buffer + ((min_pt.y>>SPS) * pitch);

		for (pt.y = min_pt.y; pt.y <= max_pt.y; pt.y += SUB_PIXEL_POW2) {
			s32 w0_col = w0_row;
			s32 w1_col = w1_row;
			s32 w2_col = w2_row;
			for (pt.x = min_pt.x; pt.x <= max_pt.x; pt.x += SUB_PIXEL_POW2) {
				// ccw vertex winding order
				if ((w0_col | w1_col | w2_col) >= 0) {
					r32 b0 = (r32)(w0_col)*one_over_tri2d_area;
					r32 b1 = (r32)(w1_col)*one_over_tri2d_area;
					r32 b2 = (r32)(w2_col)*one_over_tri2d_area;
					r32 z = (z0 + b1*(dz10) + b2*(dz20));
					r32 w = 1.0f / (b0*w0 + b1*w1 + b2*w2);
					if (w >= 0.0f && z <= depth_buffer[pt.y>>SPS][pt.x>>SPS]) {
						Assert(0.0f <= z && z <= 1.0f);
						depth_buffer[pt.y>>SPS][pt.x>>SPS] = z;
						u32 *pixel = (u32 *)line;
						r32 u = b0*uv_v0[0] + b1*uv_v1[0] + b2*uv_v2[0];
						r32 v = b0*uv_v0[1] + b1*uv_v1[1] + b2*uv_v2[1];
						u *= w;
						v *= w;

#ifndef BILINEAR_FILTER
						s32 nu = (s32)(u*(tex_width-1) + 0.5f);
						s32 nv = (s32)(v*(tex_height-1) + 0.5f);
						u32 *t = (u32 *)texture->data + (nv*tex_width) + nu;
						pixel[pt.x>>SPS] = PackRGBA(Modulate(UnpackRGBA(*t), poly_color));
#else
						s32 nu = (s32)(u*(tex_width-2));
						s32 nv = (s32)(v*(tex_height-2));
						r32 du = (u*(tex_width-2)) - nu;
						r32 dv = (v*(tex_height-2)) - nv;
						u32 *c0 = (u32 *)texture->data + (nv*tex_width) + nu;
						u32 *c1 = (u32 *)texture->data + (nv*tex_width) + (nu+1);
						u32 *c2 = (u32 *)texture->data + ((nv+1)*tex_width) + nu;
						u32 *c3 = (u32 *)texture->data + ((nv+1)*tex_width) + (nu+1);
						Vec4 a = Lerp(UnpackRGBA(*c0), UnpackRGBA(*c1), du);
						Vec4 b = Lerp(UnpackRGBA(*c2), UnpackRGBA(*c3), du);
						Vec4 c = Lerp(a, b, dv);
						pixel[(pt.x>>SPS)] = PackRGBA(Modulate(c, poly_color));
						//pixel[(pt.x>>SPS)] = PackRGBA(poly_color);
#endif
					}
				}

				w0_col -= v12.v.y;
				w1_col -= v20.v.y;
				w2_col -= v01.v.y;
			}

			w0_row += v12.v.x;
			w1_row += v20.v.x;
			w2_row += v01.v.x;
			line += pitch;
		}
	}
}

#if 1
inline static int MapLowerAsciiToTTF(char c) {
	int result = c - 97;
	Assert(result >= 0 && result <= 25);

	return result;
}
inline static int MapHigherAsciiToTTF(char c) {
	int result = c - 65;
	Assert(result >= 0 && result <= 25);

	return result + 26;
}
#endif

// draws monochrome bitmap
static void RB_DrawGlyphs(RenderTarget *rt, Bitmap *bm, Vec2i origin) {
	int w = bm->dim[0];
	int h = bm->dim[1];
	Assert(w > 0 && h > 0);
	Assert(w < rt->width && h < rt->height);
	Assert(origin[0] >= 0 && origin[1] >= 0 && (origin[0] + w) < rt->width && (origin[1] + h) < rt->height);

	int pitch = rt->pitch;
	byte *src = bm->data;
	byte *dst = rt->frame_buffer + (pitch * origin[1]) + (origin[0] * rt->bpp);	
	u32 *src_pixel = (u32 *)src;

	for (int i = 0; i < h; i++){
		u32 *dst_pixel = (u32 *)dst;
		for (int j = 0; j < w; j++) {
			if (*src_pixel) {
				*dst_pixel = *src_pixel;
			}
			++src_pixel;
			++dst_pixel;
		}
		dst += pitch;
	}
}

#if 1
static const void *RB_DrawText(RenderTarget *rt, const void *data) {
	DrawTextCmd *cmd = (DrawTextCmd *)data;
	RB_DrawGlyphs(rt, &cmd->bitmap, cmd->origin);

	return (const void *)(cmd + 1);
}
#endif

static void RB_ClearMemset(void *buffer, size_t size) {
	memset(buffer, 0, size);
}

static void RB_Blit(HDC hdc, HDC hdc_dib, Vec2i min_xy, Vec2i max_xy) {
	BitBlt(hdc, min_xy[0], min_xy[1], max_xy[0], max_xy[1], hdc_dib, 0, 0, SRCCOPY);
}

static const void *RB_DrawMesh(RenderTarget *rt, const void *data) {
	DrawPolyCmd *cmd = (DrawPolyCmd *)data;

	RB_DrawSolidMesh(cmd->polys, rt->frame_buffer, rt->depth_buffer, &cmd->texture, rt->pitch, rt->bpp, rt->width, rt->height, cmd->num_polys);
	//if (cmd->is_wireframe) {
	//	RB_DrawWireframeMesh(cmd->polys, cmd->poly_verts, rt->buffer, rt->pitch, rt->bpp, rt->width, rt->height, cmd->num_polys);
	//} else {
	//}

	return (const void *)(cmd + 1);
}

static const void *RB_SwapBuffers(RenderTarget *rt, const void *data) {
	SwapBuffersCmd *cmd = (SwapBuffersCmd *)data;
	RB_Blit(cmd->hdc, cmd->hdc_dib_section, MakeVec2i(0, 0), MakeVec2i(rt->width, rt->height));

	return (const void *)(cmd + 1);
}

static const void *RB_ClearBuffers(RenderTarget *rt, const void *data) {
	ClearBufferCmd *cmd = (ClearBufferCmd *)data;
	RB_ClearMemset(rt->frame_buffer, rt->pitch * rt->height);

	// TODO: Memset.
	for (int i = 0; i < rt->height; i++) {
		for (int j = 0; j < rt->width; j++) {
			rt->depth_buffer[i][j] = 1.0f;
		}
	}

	return (const void *)(cmd + 1);
}

void RB_ExecuteRenderCommands(RenderTarget *rt, const void *data) {
	for (;;) {
		switch (*(const int *)data) {
		case RCMD_CLEAR:
			data = RB_ClearBuffers(rt, data);
			break;
		case RCMD_SWAP_BUFFERS:
			data = RB_SwapBuffers(rt, data);
			break;
		case RCMD_TEXT:
			data = RB_DrawText(rt, data);
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
