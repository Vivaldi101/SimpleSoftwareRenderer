#ifndef KEYS_H
#define KEYS_H
#include "shared.h"
#include "keycodes.h"

#define	MAX_KEYS 256

struct Key {
	b32		down;
	int		repeats;		// if > 1, it is autorepeating
	char	*binding;
};

//extern b32	key_overstrikeMode;
extern Key	global_keys[MAX_KEYS];


#endif	// Header guard