#include "keys.h"
#include "common.h"

void IN_ClearKeys(Input *in) {
	for (int i = 0; i < MAX_NUM_KEYS; ++i) {
		in->keys[i].pressed = 0;
		in->keys[i].released = 0;
		in->keys[i].down = 0;
		in->keys[i].repeats = 0;
	}
}

static void IN_HandleKeyDown(Key *k, b32 down) {
	b32 was_down = k->down;
	k->pressed = !was_down && down;
	k->released = was_down && !down;
	k->down = down;
}

void IN_UpdateKeyboard(Input *in) {
	BYTE kb[MAX_NUM_KEYS];
	Assert(GetKeyboardState(kb));

	for (int i = 0; i < MAX_NUM_KEYS; ++i) {
		IN_HandleKeyDown(in->keys + i, kb[i] >> 7);
	}
}
