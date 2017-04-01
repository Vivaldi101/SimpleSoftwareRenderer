#ifndef KEYS_H
#define KEYS_H
#include "shared.h"
#include "keycodes.h"

struct Key {
	b32		pressed;	// edge condition, !down -> down
	b32		released;	// edge condition, down -> !down
	b32		down;		// current status 		
	u32		repeats;		
};


#endif	// Header guard