#include <Windows.h>
#include "common.h"
#include <stdio.h>

/***
**** X-routines
****/
XFunc(Pyramid) {
	Pyramid_ *pp = (Pyramid_ *)_raw_entity_data_;
	MessageBoxA(0, "NPC(pyr)\n", 0, 0);
}

XFunc(Cube) {
	Cube_ *fp = (Cube_ *)_raw_entity_data_;
	switch(_extra_flags_) {
		case PLAYER: {
			MessageBoxA(0, "Player(cube)\n", 0, 0);
		} break; 
		case NPC: {
			MessageBoxA(0, "NPC(cube)\n", 0, 0);
		} break; 
		default: Assert(0);
	}
}

static size_t GetSize(EntityEnum ee) {
#define X(name) case(name): { return sizeof(name##_); }
	switch(ee) { XMACRO }
#undef X

	return 0;
}

void UpdateEntities(BaseEntity *root_be) {
#define X(name) case(name): Update##name(raw_entity_data, extra_flags); break;
	for (BaseEntity *p = root_be; p; p = p->next) {
		EntityEnum ee = p->type;
		int num_entities = p->num_entities;
		int extra_flags = p->extra_flags;
		byte *raw_entity_data = (byte *)p->raw_entity_data;
		for (int i = 0; i < num_entities; i++) {
			switch(ee) { XMACRO }
			raw_entity_data += GetSize(ee);
		}
	}	
#undef X
}

static size_t AddEntities(BaseEntity **root_be, size_t *used_entity_memory, int num_entities, EntityEnum ee, u8 extra_flags) {
	BaseEntity *curr_be, *new_be;
	byte *curr_pos;

	curr_pos = (byte *)*root_be;
	while ((curr_be = (*root_be))) {
		curr_pos += (sizeof(BaseEntity) + (GetSize(curr_be->type) * curr_be->num_entities));
		root_be = &curr_be->next;
	}

	new_be = (BaseEntity *)curr_pos;
	new_be->next = curr_be;
	new_be->type = ee;
	new_be->num_entities = num_entities;
	new_be->extra_flags = extra_flags;
	new_be->raw_entity_data = curr_pos + sizeof(BaseEntity);
	*root_be = new_be;

	*used_entity_memory += (num_entities * GetSize(ee));
	return num_entities * GetSize(ee);
}

void InitEntities(Platform *pf, size_t max_entity_memory_limit) {
	size_t used_entity_memory = 0;
	int num_players = 1;
	int num_npcs_cubes = 3;
	int num_npcs_pyrs = 3;

	byte *prev_pos = (byte *)GetMemStackPos(&pf->main_memory_stack.perm_data);
	pf->game_state->entities = (BaseEntity *)prev_pos;
	memset(pf->game_state->entities, 0, sizeof(*pf->game_state->entities));
	AddEntities(&pf->game_state->entities, &used_entity_memory, num_npcs_cubes, Cube, NPC);
	AddEntities(&pf->game_state->entities, &used_entity_memory, num_players, Cube, PLAYER);
	AddEntities(&pf->game_state->entities, &used_entity_memory, num_npcs_pyrs, Pyramid, NPC);
	PushArray(&pf->main_memory_stack.perm_data, used_entity_memory, byte);	// push the entities onto the stack
	byte *curr_pos = (byte *)GetMemStackPos(&pf->main_memory_stack.perm_data);
	Assert(used_entity_memory <= max_entity_memory_limit);
}
