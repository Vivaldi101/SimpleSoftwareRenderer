#ifndef ENTITY_H
#define ENTITY_H
#include "shared.h"

#define SetupEntity(table, e, fn) (table)[EntityTypeEnum::##e].fn = (fn##e)
enum EntityTypeEnum {
	Invalid,
	Player,
	MAX_NUM_ENTITY_TYPES
};

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
#endif	// Header guard
