#include "shared.h"

Cvar	*global_cvar_vars;
Cvar	*global_cvar_cheats;
int		 global_cvar_modified_flags;

#define	MAX_CVARS 1024
Cvar	global_cvar_indexes[MAX_CVARS];
int		global_cvar_num_indexes;

#define FILE_HASH_SIZE 256
static Cvar *global_hashable[FILE_HASH_SIZE];