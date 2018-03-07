#include "r_cmds.h"
#include "renderer.h"

#define SUB_PIXEL_STEP 8
#define SPS SUB_PIXEL_STEP
#define SUB_PIXEL_POW2 (1<<SUB_PIXEL_STEP)
#define SUB_PIXEL_POW2_MINUS_1 (1<<(SUB_PIXEL_STEP-1))
#define SUB_PIXEL_OFFSET ((1<<SUB_PIXEL_STEP) >> 1)
#define SPS_OFFSET SUB_PIXEL_OFFSET
#define lerp(s, d, t) ((1.0f-(t))*(s) + (t)*(d))


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

// FIXME: atm the drawing routines receive pitch and bpp, 
// will remove the concept completely at some point 
// FIXME: these determinant predicates are here for the time being
// ccw vertex winding order
static s32 Orient2D(Point2D a, Point2D b, Point2D p) {
   s64 result;
   s64 ab = a.y-b.y;
   s64 ba = b.x-a.x;
   s64 c = (s64)((s64)a.x*(s64)b.y) - (s64)((s64)a.y*(s64)b.x);

   //result = (s64)((s64)(((s64)b.x-a.x)*(s64)((s64)p.y-a.y)+128) - (s64)(((s64)b.y-a.y)*(s64)((s64)p.x-a.x))+128);
   result = p.x*ab + p.y*ba + c;
   result += (1<<(SPS-1));
   return (s32)(result>>SPS);
}

static s32 Tri2Area2d(Vec2i a, Vec2i b) {
   s64 result = (s64)((s64)a.v.x*(s64)b.v.y) - (s64)((s64)a.v.y*(s64)b.v.x);

   return (s32)(result>>SPS);
}

// FIXME: these determinant predicates are here for the time being
// ccw vertex winding order
static inline s32 Orient2D(s32 ax, s32 ay, s32 bx, s32 by, s32 cx, s32 cy) {
	Point2D a = {ax, ay};
	Point2D b = {bx, by};
	Point2D c = {cx, cy};
	return Orient2D(a, b, c);
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

#if 0
#define MapAsciiToTTF(c) (c) - 65
#else
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


// FIXME: change width and height to render_target_*
static void RB_DrawSolidMesh(Poly *polys, byte *buffer, int pitch, int bpp, int width, int height, int num_polys) {
	for (int i = 0; i < num_polys; i++) {
		if ((polys[i].state & POLY_STATE_BACKFACE)) {
			continue;
		}

      u32 poly_color = polys[i].color;
      Vec4 red = {1.0f,0.0f,0.0f,1.0f};
      Vec4 green = {0.0f,1.0f,0.0f,1.0f};
      Vec4 blue = {0.0f,0.0f,1.0f,1.0f};

		// original vertices
		PolyVert v0 = polys[i].vertex_array[0];
		PolyVert v1 = polys[i].vertex_array[1];
		PolyVert v2 = polys[i].vertex_array[2];

		s32 x0 = RoundReal32ToS32(v0.xyz.v.x * SUB_PIXEL_POW2);
		s32 y0 = RoundReal32ToS32(v0.xyz.v.y * SUB_PIXEL_POW2);
		r32 z0 = v0.xyz.v.z;

		s32 x1 = RoundReal32ToS32(v1.xyz.v.x * SUB_PIXEL_POW2);
		s32 y1 = RoundReal32ToS32(v1.xyz.v.y * SUB_PIXEL_POW2);
		r32 z1 = v1.xyz.v.z;

		s32 x2 = RoundReal32ToS32(v2.xyz.v.x * SUB_PIXEL_POW2);
		s32 y2 = RoundReal32ToS32(v2.xyz.v.y * SUB_PIXEL_POW2);
		r32 z2 = v2.xyz.v.z;

      // compute triangle 2d AABB for the scaled points
		int min_x = (MIN3(x0, x1, x2));
		int min_y = (MIN3(y0, y1, y2));

		int max_x = (MAX3(x0, x1, x2));
		int max_y = (MAX3(y0, y1, y2));

		Point2D min_pt = {(min_x >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP, (min_y >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP};
		min_pt.x = min_pt.x + SUB_PIXEL_OFFSET;
		min_pt.y = min_pt.y + SUB_PIXEL_OFFSET;
		Point2D max_pt = {(max_x >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP, (max_y >> SUB_PIXEL_STEP) << SUB_PIXEL_STEP};
		max_pt.x = max_pt.x + SUB_PIXEL_OFFSET; 
		max_pt.y = max_pt.y + SUB_PIXEL_OFFSET;


		// clip against the screen bounds
		//min_pt.x = MAX(min_pt.x, 0);
		//min_pt.y = MAX(min_pt.y, 0);

		//max_pt.x = MIN(max_pt.x, ((((width-1))   << SPS) - SUB_PIXEL_OFFSET));
		//max_pt.y = MIN(max_pt.y, ((((height-1))	<< SPS) - SUB_PIXEL_OFFSET));

		// clip against the screen bounds
      min_pt.x = MAX(min_pt.x, ((-width/2)<<SPS) - SPS_OFFSET);
		min_pt.y = MAX(min_pt.y, ((-height/2)<<SPS) - SPS_OFFSET);

		max_pt.x = MIN(max_pt.x, ((((width-1)/2)<<SPS) - SPS_OFFSET));
		max_pt.y = MIN(max_pt.y, ((((height-1)/2)<<SPS) - SPS_OFFSET));

		// make separate point structures for edge testing
		Point2D pv0 = {x0, y0};
		Point2D pv1 = {x1, y1};
		Point2D pv2 = {x2, y2};

		//s32 tri2d_area = Orient2D(pv0, pv1, pv2);
      Vec2i a = {pv1.x-pv0.x, pv1.y-pv0.y};
      Vec2i b = {pv2.x-pv0.x, pv2.y-pv0.y};
      s32 tri2d_area = Tri2Area2d(a, b);
      if (tri2d_area <= 0) {                          // degenerate or back-facing triangle
         continue;
      }
      //if (z0 < 0.0f && z1 < 0.0f && z2 < 0.0f) {      // FIXME: is this needed?
      //   continue;
      //}

      // FIXME: macro for constants
      if (x0 > (1043<<SPS) || y0 > (1043<<SPS)) {     // FIXME: guard band clip region needed
         continue;
      }
      if (x1 > (1043<<SPS) || y1 > (1043<<SPS)) {     // FIXME: guard band clip region needed
         continue;
      }
      if (x2 > (1043<<SPS) || y2 > (1043<<SPS)) {     // FIXME: guard band clip region needed
         continue;
      }

      if (x0 < (-1024<<SPS) || y0 < (-1024<<SPS)) {     // FIXME: guard band clip region needed
         continue;
      }
      if (x1 < (-1024<<SPS) || y1 < (-1024<<SPS)) {     // FIXME: guard band clip region needed
         continue;
      }
      if (x2 < (-1024<<SPS) || y2 < (-1024<<SPS)) {     // FIXME: guard band clip region needed
         continue;
      }

      r32 one_over_tri2d_area = 1.0f / (r32)tri2d_area;

		// compute biases for top-left fill convention
		int bias0 = IsTopLeftEdge(pv1, pv2) ? 0 : -1;
		int bias1 = IsTopLeftEdge(pv2, pv0) ? 0 : -1;
		int bias2 = IsTopLeftEdge(pv0, pv1) ? 0 : -1;

		// positive edge test if pt is to the left of the directed line segment
		s32 w0_row = (Orient2D(pv1, pv2, min_pt)) + bias0;
		s32 w1_row = (Orient2D(pv2, pv0, min_pt)) + bias1;
		s32 w2_row = (Orient2D(pv0, pv1, min_pt)) + bias2;

		// compute delta terms for the edge functions for the scaled points
		s32 a01 = ((y0 - y1)), b01 = ((x1 - x0)); 
		s32 a12 = ((y1 - y2)), b12 = ((x2 - x1));
		s32 a20 = ((y2 - y0)), b20 = ((x0 - x2));

		r32 dz10 = z1-z0;
      r32 dz20 = z2-z0;

		// point to test against the triangle
		Point2D pt;

		min_pt.x = ((min_pt.x+(1<<SUB_PIXEL_STEP)) >> SUB_PIXEL_STEP); 
		min_pt.y = ((min_pt.y+(1<<SUB_PIXEL_STEP)) >> SUB_PIXEL_STEP); 
		max_pt.x = ((max_pt.x+(1<<SUB_PIXEL_STEP)) >> SUB_PIXEL_STEP); 
		max_pt.y = ((max_pt.y+(1<<SUB_PIXEL_STEP)) >> SUB_PIXEL_STEP); 


      min_pt.x += (width>>1);
      min_pt.y += (height>>1);
      max_pt.x += (width>>1);
      max_pt.y += (height>>1);
      byte *line = buffer + (min_pt.y * pitch);

		for (pt.y = min_pt.y; pt.y <= max_pt.y; pt.y++) {
			int w0_col = w0_row;
			int w1_col = w1_row;
			int w2_col = w2_row;
			for (pt.x = min_pt.x; pt.x <= max_pt.x; pt.x++) {
            // ccw vertex winding order
            if ((w0_col | w1_col | w2_col) >= 0) {
               r32 b0 = (r32)(w0_col)*one_over_tri2d_area;
               r32 b1 = (r32)(w1_col)*one_over_tri2d_area;
               r32 b2 = (r32)(w2_col)*one_over_tri2d_area;
               r32 z = (z0 + b1*(dz10) + b2*(dz20));
               if (z >= 0.0f && z <= 1.0f) {
                  u32 *pixel = (u32 *)line;
                  Vec4 c = b0*red + b1*green + b2*blue;
                  //pixel[pt.x] = PackRGBA(c);
                  //Vec4 l = lerp(UnpackRGBA(poly_color), MakeVec4(0.0f, 0.0f, 0.0f, 1.0f), (Square(z*z*z)));    // fog testing
                  pixel[pt.x] = PackRGBA(c);
                  //pixel[pt.x] = poly_color;
               }
            }
				w0_col += a12;
				w1_col += a20;
				w2_col += a01;
			}
			w0_row += b12;
			w1_row += b20;
			w2_row += b01;
			line += pitch;
		}
	}
}

// FIXME: change width and height to render_target_*



static void RB_DrawRect(RenderTarget *rt, Bitmap *bm, Vec2 origin) {
	int w = bm->dim[0];
	int h = bm->dim[1];
	Assert(w < rt->width && h < rt->height);
	Assert(origin[0] >= 0 && origin[1] >= 0 && (origin[0] + w) < rt->width && (origin[1] + h) < rt->height);

	int pitch = rt->pitch;
	byte *src = bm->data;
	byte *dst = rt->buffer + (pitch * (int)(origin[1] + 0.5f)) + ((int)(origin[0] + 0.5f) * rt->bpp);	
	u32 *src_pixel = (u32 *)src;

	for (int i = 0; i < h; i++){
		u32 *dst_pixel = (u32 *)dst;
		for (int j = 0; j < w; j++) {
			*dst_pixel++ = (*src_pixel);
			++src_pixel;
		}
		dst += pitch;
	}
}

static const void *RB_DrawText(RenderTarget *rt, const void *data) {
	DrawTextCmd *cmd = (DrawTextCmd *)data;
	Vec2 o = cmd->basis.origin;
	for (char i = *cmd->text; i = *cmd->text; ++cmd->text) {
		if (i != ' ') {
			int k = (i >= 97) ? MapLowerAsciiToTTF(i) : MapHigherAsciiToTTF(i);
			RB_DrawRect(rt, &cmd->bitmap[k], o);
			o[0] += cmd->bitmap[k].dim[0];
		} else {
			o[0] += 10.0f;
		}
	}

	return (const void *)(cmd + 1);
}

static void RB_ClearMemset(void *buffer, size_t size) {
	memset(buffer, 0, size);
}

static void RB_Blit(HDC hdc, HDC hdc_dib, Vec2i min_xy, Vec2i max_xy) {
	BitBlt(hdc, min_xy[0], min_xy[1], max_xy[0], max_xy[1], hdc_dib, 0, 0, SRCCOPY);
}

static const void *RB_DrawMesh(RenderTarget *rt, const void *data) {
	DrawPolyCmd *cmd = (DrawPolyCmd *)data;

	RB_DrawSolidMesh(cmd->polys, rt->buffer, rt->pitch, rt->bpp, rt->width, rt->height, cmd->num_polys);
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
				//data = RB_DrawRect(rt, data);
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
