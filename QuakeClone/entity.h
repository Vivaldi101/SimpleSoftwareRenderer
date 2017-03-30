#ifndef ENTITY_H
#define ENTITY_H
#include "shared.h"

#define SetupEntity(table, e, fn) (table)[EntityTypeEnum::##e].fn = (fn##e)
enum EntityTypeEnum {
	Invalid,
	Player,
	MAX_NUM_ENTITY_TYPES
};

struct Entity {
	EntityTypeEnum	type;

	union {
		struct {
			Vec3	velocity, dir;
		} player;
	};
};

struct DispatchTable {
    void (*Update)(Entity*, r32);
    void (*Draw)(Entity*);
};
#endif	// Header guard
