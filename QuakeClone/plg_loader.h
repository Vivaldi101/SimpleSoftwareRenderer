#ifndef PLG_LOADER_H
#define PLG_LOADER_H

#include "shared.h"
#include "renderer_local.h"
#include "win_local.h"

// just for time being the func takes the general polygon mesh and the player
extern void	PLG_InitParsing(const char *plg_file_name, MeshObject *md, MeshObject *player_md);
#endif	// header guard
