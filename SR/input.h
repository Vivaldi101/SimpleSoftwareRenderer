#ifndef INPUT_H
#define INPUT_H
#include "shared.h"
#include "keycodes.h"

struct Key {
	b32		pressed;	   // edge condition, !down -> down
	b32		released;	// edge condition, down -> !down
	b32		down;		   // current status 		
	u32		repeats;		
};

struct Mouse {
	Key		left_button;
	Key		right_button;
	Vec2	window_pos;
	Vec2	screen_pos;
	Vec2	pos_delta;
};

#endif	// Header guard