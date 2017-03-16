#ifndef PLG_LOADER_H
#define PLG_LOADER_H

// FIXME: move into files.cc
#include "shared.h"
#include "renderer_local.h"
#include "win_local.h"

extern b32 PLG_LoadMeshObject(MeshObject *md, FILE **fp, Vec3 world_pos, r32 scale = 1.0f);
#endif	// header guard
