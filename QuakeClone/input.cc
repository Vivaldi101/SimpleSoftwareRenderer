#include "keys.h"
#include "common.h"
//b32	global_any_key_down;

void IN_ClearKeyStates(Input *in) {
	for (int i = 0; i < MAX_NUM_CONTROLLERS; ++i) {
		for (int j = 0; j < MAX_NUM_KEYS; ++j) {
			in->controllers[i].buttons[j].pressed = 0;
			in->controllers[i].buttons[j].repeats = 0;
		}
	}
}

void IN_HandleControllerEvent(int key) {
}

int IN_MapKeyToIndex(Input *in, int key, b32 pressed, u32 time) {
	int mapped_key = ControllerIndexEnum::KEY_INVALID;

	// FIXME: make into switch
	// FIXME: handle all the controllers
	if (key == VK_UP) {
		in->controllers[0].up.pressed = pressed;
		mapped_key = ControllerIndexEnum::KEY_UP;
	} else if (key == VK_DOWN) {
		in->controllers[0].down.pressed = pressed;
		mapped_key = ControllerIndexEnum::KEY_DOWN;
	} else if (key == VK_LEFT) {
		in->controllers[0].left.pressed = pressed;
		mapped_key = ControllerIndexEnum::KEY_LEFT;
	} else if (key == VK_RIGHT) {
		in->controllers[0].right.pressed = pressed;
		mapped_key = ControllerIndexEnum::KEY_RIGHT;
	} else if (key == VK_SPACE) {
		in->controllers[0].space.pressed = pressed;
		mapped_key = ControllerIndexEnum::KEY_SPACE;
	} else if (key == VK_PAUSE) {
		in->controllers[0].pause.pressed = pressed;
		mapped_key = ControllerIndexEnum::KEY_PAUSE;
	} 

	if (pressed) {
		in->controllers[0].buttons[mapped_key].repeats++;
	} else {
		in->controllers[0].buttons[mapped_key].repeats = 0;
	}

	return mapped_key;
}

b32 IN_IsKeyDown(Input *in, int key) {
	// FIXME: handle all the controllers
	return in->controllers[0].buttons[key].pressed;
}
