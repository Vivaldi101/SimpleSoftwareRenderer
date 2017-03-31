#include "shared.h"
#include "renderer.h"

#define GetRenderCmdType(cmds, t) (t *)(R_GetCommandBuffer)((cmds), sizeof(t)) 

void *R_GetCommandBuffer(RenderCommands *rc, u32 num_bytes) {
	byte *cmd_buffer = rc->buffer_base;

	// leave room for the end of list command
	if (rc->used_buffer_size + num_bytes + sizeof(int) > rc->max_buffer_size) {
		// drop the current command if overflowing
		return 0;
	}
	rc->used_buffer_size += num_bytes;

	return cmd_buffer + rc->used_buffer_size - num_bytes;
}

void R_IssueRenderCommands(RenderCommands *rc) {
	byte *cmd_buffer = rc->buffer_base;
	*(int *)(cmd_buffer + rc->used_buffer_size) = RCMD_END_OF_CMDS;

	rc->used_buffer_size = 0;
	RB_ExecuteRenderCommands(rc->buffer_base);
}

// FIXME: maybe pass only num of meshes, polys and transformed verts
void R_DrawMesh(VidSystem *vs, RenderCommands *rc, Poly *polys, Vec3 *poly_verts, int num_polys, b32 solid) {
	DrawMeshCmd *cmd = GetRenderCmdType(rc, DrawMeshCmd);

	cmd->cmd_id = RCMD_MESH;
	cmd->buffer = vs->buffer;
	cmd->polys = polys;
	cmd->poly_verts = poly_verts;
	cmd->num_polys = num_polys;
	cmd->pitch = vs->pitch;
	cmd->bpp = vs->bpp;
	cmd->color = polys->color;
	cmd->width = vs->width;
	cmd->height = vs->height;
	cmd->solid = solid;
}
void R_BeginFrame(VidSystem *vs, RenderCommands *rc) {
	ClearBufferCmd *cmd = GetRenderCmdType(rc, ClearBufferCmd);

	cmd->cmd_id = RCMD_CLEAR;
	cmd->buffer = vs->buffer;
	cmd->pitch = vs->pitch;
	cmd->height = vs->height;
}

void R_EndFrame(VidSystem *vs, RenderCommands *rc) {
	SwapBuffersCmd *cmd = GetRenderCmdType(rc, SwapBuffersCmd);

	if (!cmd) {
		return;
	}
	cmd->cmd_id = RCMD_SWAP_BUFFERS;
	cmd->hdc = vs->win_handles.hdc;
	cmd->hdc_dib_section = vs->win_handles.hdc_dib_section;
	cmd->width = vs->width;
	cmd->height = vs->height;
	R_IssueRenderCommands(rc);
}
