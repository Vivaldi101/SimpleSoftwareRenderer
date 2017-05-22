#ifndef PLG_LOADER_H
#define PLG_LOADER_H

#include "shared.h"
#include "renderer.h"

extern b32 PLG_LoadMesh(Entity *typeless_ent, const void *load_data, u32 load_data_len, r32 scale = 1.0f);
#endif	// Header guard
