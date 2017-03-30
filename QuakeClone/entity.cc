#include "entity.h"
void Update(Entity* entity, r32 dt) {
	//global_dpt[entity->type].Update(entity, dt);
}

void Draw(Entity* entity) {
	//global_dpt[entity->type].Draw(entity);
}

void SetupEntityTable(DispatchTable *dpt) {
	//SetupEntity(dpt, Player, Update);
	//SetupEntity(dpt, Player, Draw);

	//SetupEntity(dpt, NPC, Update);
	//SetupEntity(dpt, NPC, Draw);
}
