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

void R_IssueRenderCommands(RenderCommands *rc) {
	byte *cmd_buffer = rc->buffer_base;
	*(s32 *)(cmd_buffer + rc->used_buffer_size) = RCMD_END_OF_CMDS;

	RB_ExecuteRenderCommands(rc->buffer_base);
	rc->used_buffer_size = 0;
}

void R_PushBitmapCmd(RenderTarget *vs, RenderCommands *rc, Dim2d d2, Vec4 color) {
	DrawBitmapCmd *cmd = PushRenderCmd(rc, DrawBitmapCmd);
	Assert(cmd);

	u32 packed_color = (roundReal32ToU32(color.c.a * 255.0f) << 24 |
						roundReal32ToU32(color.c.r * 255.0f) << 16 |
						roundReal32ToU32(color.c.g * 255.0f) << 8  |
				 		roundReal32ToU32(color.c.b * 255.0f));

	cmd->cmd_id = RCMD_BITMAP;
	cmd->d2.width = d2.width;
	cmd->d2.height = d2.height;
	cmd->color = packed_color;
}

void R_PushPolysCmd(RenderTarget *vs, RenderCommands *rc, Poly *polys, Vec3 *poly_verts, int num_polys, b32 is_wireframe) {
	DrawPolyListCmd *cmd = PushRenderCmd(rc, DrawPolyListCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_MESH;
	cmd->buffer = vs->buffer;
	cmd->polys = polys;
	cmd->poly_verts = poly_verts;
	cmd->num_polys = num_polys;
	cmd->pitch = vs->pitch;
	cmd->bpp = vs->bpp;
	cmd->width = vs->width;
	cmd->height = vs->height;
	cmd->is_wireframe = is_wireframe;
}

void R_BeginFrame(RenderTarget *vs, RenderCommands *rc) {
	ClearBufferCmd *cmd = PushRenderCmd(rc, ClearBufferCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_CLEAR;
	cmd->buffer = vs->buffer;
	cmd->size = vs->pitch * vs->height;
}

void R_EndFrame(RenderTarget *vs, RenderCommands *rc) {
	SwapBuffersCmd *cmd = PushRenderCmd(rc, SwapBuffersCmd);
	Assert(cmd);

	cmd->cmd_id = RCMD_SWAP_BUFFERS;
	cmd->hdc = vs->win_handles.hdc;
	cmd->hdc_dib_section = vs->win_handles.hdc_dib_section;
	cmd->width = vs->width;
	cmd->height = vs->height;
	R_IssueRenderCommands(rc);
}
