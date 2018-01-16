#ifndef PLG_LOADER_H
#define PLG_LOADER_H

#include "shared.h"
#include "renderer.h"
#include "entity.h"

extern b32 PLG_LoadMesh(BaseEntity *typeless_ent, const void *load_data, u32 load_data_len, r32 scale = 1.0f);
#endif	// Header guard
