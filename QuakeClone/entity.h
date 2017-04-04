#ifndef ENTITY_H
#define ENTITY_H
#if 0
#include "shared.h"


struct Entity2 {
	EntityTypeEnum	type;

	union {
		struct {
			Vec3	velocity, dir;
		} player;
	};
};

struct DispatchTable {
    void (*Update)(Entity2*, r32);
    void (*Draw)(Entity2*);
};
#endif	
#endif	// Header guard
