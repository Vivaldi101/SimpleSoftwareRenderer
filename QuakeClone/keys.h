#ifndef KEYS_H
#define KEYS_H
#include "shared.h"
#include "keycodes.h"

#define	MAX_NUM_KEYS 256

struct Key {
	b32		pressed;
	u32		repeats;		
	char *	binding;
};


#endif	// Header guard