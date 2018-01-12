#include "entity.h"

/***
**** X-routines
****/

// usage: XFunc(MyType) { 
//     MyType *p = (MyType *)_data_;
// }

XFunc(Cube) {
	Cube_ *fp = (Cube_ *)_data_;
	MessageBoxA(0, "Cube\n", 0, 0);
}

static void *AdvanceBySizeof(EntityEnum size_to_advance, void *start) {
#define X(name) case(name): { return (end += sizeof(name##_)); }
	byte *end = (byte *)start;
	switch(size_to_advance) { XMACRO }
#undef X

	Assert(0);	// invalid case
	return 0;
}

static void UpdateEntities(byte *buffer, int num_entities) {
	void *start, *end;
#define X(name) case(name): Update##name(((BaseEntity *)start)->data); break;
	for (int i = 0; i < num_entities; i++) {
		start = buffer;
		switch(((BaseEntity *)start)->type) { XMACRO }
		end = (byte *)start + sizeof(BaseEntity);
		buffer = (byte *)AdvanceBySizeof(((BaseEntity *)start)->type, end);
	}
#undef X
}


// for entity array types
#define X(name) + 1
static const int NUM_ENTITY_TYPES = (0 + XMACRO); 
#undef X

byte *AddEntity(byte *buffer, EntityEnum ee) {
	byte *old_buffer;
	BaseEntity *be = (BaseEntity *)buffer;
	be->type = ee; 
	buffer += sizeof(BaseEntity);
	be->data = (byte *)buffer;
	old_buffer = buffer;
	buffer = (byte *)AdvanceBySizeof(be->type, buffer);	

	return buffer;
}

byte *CreateEntities(byte *entity_buffer) {

	// create some cubes
	int num_cubes = 3;
	byte *curr_pos = entity_buffer;
	for (int i = 0; i < num_cubes; i++) {
		curr_pos = AddEntity(curr_pos, Cube);
		//Assert((ptrdiff_t)base_memory_addr < ((ptrdiff_t)start_addr+(ptrdiff_t)ENTITY_MEMORY_SIZE));
	}

	// in sim loop
	//UpdateEntities(entity_buffer, num_cubes);

	return curr_pos;
}
