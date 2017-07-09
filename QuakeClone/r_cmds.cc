#include "shared.h"
#include "renderer.h"

#define PushRenderCmd(cmds, t) (t *)(R_PushRenderCmd)((cmds), sizeof(t)) 

static void *R_PushRenderCmd(RenderCommands *rc, u32 num_bytes) {
	byte *cmd_buffer = rc->buffer_base;

	// leave room for the end of list command
	if ((rc->used_buffer_size + num_bytes + sizeof(s32)) > rc->max_buffer_size) {
		// drop the current command if overflowing
		return 0;
	}
	rc->used_buffer_size += num_bytes;

	return cmd_buffer + rc->used_buffer_size - num_bytes;
}

void R_IssueRenderCommands(RenderTarget *rt, RenderCommands *rc) {
	byte *cmd_buffer = rc->buffer_base;
	*(s32 *)(cmd_buffer + rc->used_buffer_size) = RCMD_END_OF_CMDS;

	RB_ExecuteRenderCommands(rt, rc->buffer_base);
	rc->used_buffer_size = 0;
}

void R_PushRectCmd(RenderCommands *rc, Bitmap bm, Vec2 origin, r32 scale, Vec4 color) {
	DrawRectCmd *cmd = PushRenderCmd(rc, DrawRectCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_RECT;
	cmd->basis.axis[0] = MakeVec2(1.0f, 0.0f) * scale;	// FIXME: 3d homogeneous matrix
	cmd->basis.axis[1] = MakeVec2(0.0f, 1.0f) * scale;
	cmd->basis.origin = origin;

	//cmd->points[0][0] = 0.0f;
	//cmd->points[0][1] = 0.0f;

	//cmd->points[1][0] = 0.0f;
	//cmd->points[1][1] = (r32)bm.d2.height;

	//cmd->points[2][0] = (r32)bm.d2.width;
	//cmd->points[2][1] = (r32)bm.d2.height;

	//cmd->points[3][0] = (r32)bm.d2.width;
	//cmd->points[3][1] = 0.0f;
	
	cmd->bitmap = bm;
	cmd->dim = bm.dim;
	cmd->color = PackRGBA(color);
}

void R_PushTextCmd(RenderCommands *rc, const char *text, Bitmap *bm, Vec2 origin, r32 scale, Vec4 color) {
	DrawTextCmd *cmd = PushRenderCmd(rc, DrawTextCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_TEXT;
	cmd->basis.axis[0] = MakeVec2(1.0f, 0.0f) * scale;	// FIXME: 3d homogeneous matrix
	cmd->basis.axis[1] = MakeVec2(0.0f, 1.0f) * scale;
	cmd->basis.origin = origin;
	cmd->bitmap = bm;
	cmd->dim = bm->dim;
	cmd->text = text;
	cmd->color = PackRGBA(color);
}

void R_PushPolysCmd(RenderCommands *rc, Poly *polys, PolyVert *poly_verts, int num_polys, b32 is_wireframe) {
	DrawPolyCmd *cmd = PushRenderCmd(rc, DrawPolyCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_MESH;
	cmd->polys = polys;
	cmd->poly_verts = poly_verts;
	cmd->num_polys = num_polys;
	cmd->is_wireframe = is_wireframe;
}

void R_BeginFrame(RenderTarget *rt, RenderCommands *rc) {
	ClearBufferCmd *cmd = PushRenderCmd(rc, ClearBufferCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_CLEAR;
	cmd->size = rt->pitch * rt->height;
}

void R_EndFrame(RenderTarget *rt, RenderCommands *rc) {
	SwapBuffersCmd *cmd = PushRenderCmd(rc, SwapBuffersCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_SWAP_BUFFERS;
	cmd->hdc = rt->win_handles.hdc;
	cmd->hdc_dib_section = rt->win_handles.hdc_dib_section;
	R_IssueRenderCommands(rt, rc);
}
