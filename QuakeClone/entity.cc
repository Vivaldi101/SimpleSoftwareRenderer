#include "entity.h"
#include "shared.h"

// will get rounded up to the next 4k page boundary on Win64
#undef ENTITY_MEMORY_SIZE 
// arbitrary value
#define ENTITY_MEMORY_SIZE 4096*2
#if ENTITY_MEMORY_SIZE <= 0
#error "ENTITY_MEMORY_SIZE must be positive nonzero number!"
#endif

#define INVALID_XMACRO_CASE ((EntityEnum)-1)
#define X(name) name,
#define XFunc(name) static void Update##name##_(void *_data_)

// give here the data types to use 
// suffix them with an underscore: MyType_
#define XMACRO X(First_) X(Second_) X(Third_) X(Fourth_) 
#undef X

typedef enum {
#define X(name) name,
	XMACRO
#undef X
} EntityEnum;

typedef struct {
	EntityEnum	type;
	void		*base;			
} Entity;

// types
typedef struct {
	int i1, i2, i3;
} First;

typedef struct {
	int i1, i2, i3;
	float f1, f2, f3;
} Second;

typedef struct {
	int i1, i2, i3;
} Third;

typedef struct {
	int i1, i2, i3;
	float f1, f2, f3;
} Fourth;

// x-routines

// usage: XFunc(MyType) { 
//     MyType *p = (MyType *)_data_;
// }

XFunc(First) {
	First *fp = (First *)_data_;
	//RunFirst(fp);
	MessageBoxA(0, "1\n", 0, 0);
}

XFunc(Second) {
	Second *sp = (Second *)_data_;
	//RunSecond(sp);
	MessageBoxA(0, "2\n", 0, 0);
}

XFunc(Third) {
	Third *tp = (Third *)_data_;
	//RunFirst(fp);
	MessageBoxA(0, "3\n", 0, 0);
}

XFunc(Fourth) {
	Fourth *sp = (Fourth *)_data_;
	sp->i1 = 42;
	//RunSecond(sp);
	MessageBoxA(0, "4\n", 0, 0);
}

static void UpdateTypes(Entity *ents, int num_entities) {
#define X(name) case(name): Update##name(ents[i].base); break;
	for (int i = 0; i < num_entities; i++) {
		switch(ents[i].type) { XMACRO }
	}
#undef X
}

static void *AdvanceBySizeof(EntityEnum size_to_advance, void *curr_ptr) {
#define X(name) case(name): { return (new_ptr += sizeof(name)); }
	byte *new_ptr = (byte *)curr_ptr;
	switch(size_to_advance) { XMACRO }
#undef X

	Assert(0);	// invalid case
	return 0;
}

static void InitAll(Entity *ents, byte *base_memory_addr, int num_entities) {
	byte *start_addr = base_memory_addr;

	Assert(start_addr);
	for (int i = 0; i < num_entities; i++) {
		ents[i].type = (EntityEnum)i; 
		ents[i].base = base_memory_addr;
		base_memory_addr = (byte *)AdvanceBySizeof(ents[i].type, base_memory_addr);	
		Assert((ptrdiff_t)base_memory_addr < (ptrdiff_t)start_addr+ENTITY_MEMORY_SIZE);
	}
}

// FIXME: how should we call this?
// includes test code only atm
void CommonMain(MemoryStack *ms, Entity *types) {
#define X(name) + 1
#define GLOBAL_NUM_ENTITIES (0 + XMACRO) 
	//Entity types[GLOBAL_NUM_ENTITIES];

	// FIXME: do allocation here or let the caller pass a buffer?
	byte *base_memory_addr = (byte *)VirtualAlloc(0, ENTITY_MEMORY_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	Assert(base_memory_addr);
	ms->base_ptr = (byte *)base_memory_addr;
	ms->max_size = ENTITY_MEMORY_SIZE;

	InitAll(types, base_memory_addr, GLOBAL_NUM_ENTITIES);

	UpdateTypes(types, GLOBAL_NUM_ENTITIES);
	
#undef X
}

