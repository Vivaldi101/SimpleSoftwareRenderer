#include "keys.h"
#include "common.h"
//b32	global_any_key_down;

void IN_ClearKeyStates(Input *in) {
	for (int i = 0; i < MAX_NUM_KEYS; ++i) {
		in->keys[i].down = 0;
		in->keys[i].repeats = 0;
	}
}

void IN_HandleKeyEvent(Input *in, int key, b32 down, u32 time) {
	Assert(key >= 0 && key < MAX_NUM_KEYS);

	in->keys[key].down = down;

	if (down) {
		in->keys[key].repeats++;
	} else {
		in->keys[key].repeats = 0;
	}

	if (in->curr_key != key) {
		in->prev_key = in->curr_key;
	}
	in->curr_key = key; 

#if 0
	// escape is always handled special
	if (key == K_ESCAPE && down) {
		if ( cls.keyCatchers & KEYCATCH_MESSAGE ) {
			// clear message mode
			Message_Key( key );
			return;
		}

		// escape always gets out of CGAME stuff
		if (cls.keyCatchers & KEYCATCH_CGAME) {
			cls.keyCatchers &= ~KEYCATCH_CGAME;
			//VM_Call (cgvm, CG_EVENT_HANDLING, CGAME_EVENT_NONE);
			return;
		}

		if ( !( cls.keyCatchers & KEYCATCH_UI ) ) {
			if ( cls.state == CA_ACTIVE && !clc.demoplaying ) {
				//VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_INGAME );
			}
			else {
				CL_Disconnect_f();
				S_StopAllSounds();
				//VM_Call( uivm, UI_SET_ACTIVE_MENU, UIMENU_MAIN );
			}
			return;
		}

		//VM_Call( uivm, UI_KEY_EVENT, key, down );
		return;
	}

	// console key is hardcoded, so the user can never unbind it
	if (key == '`' || key == '~') {
		if (!down) {
			return;
		}
    ConoggleConsole_f ();
		return;
	}


	// keys can still be used for bound actions
	if ( down && ( key < 128 || key == K_MOUSE1 ) && ( clc.demoplaying || cls.state == CA_CINEMATIC ) && !cls.keyCatchers) {

		if (Cvar_VariableValue ("com_cameraMode") == 0) {
			Cvar_Set ("nextdemo","");
			key = K_ESCAPE;
		}
	}



	//
	// key up events only perform actions if the game key binding is
	// a button command (leading + sign).  These will be processed even in
	// console mode and menu mode, to keep the character from continuing 
	// an action started before a mode switch.
	//
	if (!down) {
		kb = keys[key].binding;

		CL_AddKeyUpCommands( key, kb );

		if ( cls.keyCatchers & KEYCATCH_UI && uivm ) {
			//VM_Call( uivm, UI_KEY_EVENT, key, down );
		} else if ( cls.keyCatchers & KEYCATCH_CGAME && cgvm ) {
			//VM_Call( cgvm, CG_KEY_EVENT, key, down );
		} 

		return;
	}


	// distribute the key down event to the apropriate handler
	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		Console_Key( key );
	} else if ( cls.keyCatchers & KEYCATCH_UI ) {
		if ( uivm ) {
			//VM_Call( uivm, UI_KEY_EVENT, key, down );
		} 
	} else if ( cls.keyCatchers & KEYCATCH_CGAME ) {
		if ( cgvm ) {
			//VM_Call( cgvm, CG_KEY_EVENT, key, down );
		} 
	} else if ( cls.keyCatchers & KEYCATCH_MESSAGE ) {
		Message_Key( key );
	} else if ( cls.state == CA_DISCONNECTED ) {
		Console_Key( key );
	} else {
		// send the bound action
		kb = keys[key].binding;
		if ( !kb ) {
			if (key >= 200) {
				Com_Printf ("%s is unbound, use controls menu to set.\n"
					, Key_KeynumToString( key ) );
			}
		} else if (kb[0] == '+') {	
			int i;
			char button[1024], *buttonPtr;
			buttonPtr = button;
			for ( i = 0; ; i++ ) {
				if ( kb[i] == ';' || !kb[i] ) {
					*buttonPtr = '\0';
					if ( button[0] == '+') {
						// button commands add keynum and time as parms so that multiple
						// sources can be discriminated and subframe corrected
						Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", button, key, time);
						Cbuf_AddText (cmd);
					} else {
						// down-only command
						Cbuf_AddText (button);
						Cbuf_AddText ("\n");
					}
					buttonPtr = button;
					while ( (kb[i] <= ' ' || kb[i] == ';') && kb[i] != 0 ) {
						i++;
					}
				}
				*buttonPtr++ = kb[i];
				if ( !kb[i] ) {
					break;
				}
			}
		} else {
			// down-only command
			Cbuf_AddText (kb);
			Cbuf_AddText ("\n");
		}
	}
#endif
}

b32 IN_IsKeyDown(Input *in, int key) {
	Assert(key >= 0 && key < MAX_NUM_KEYS);
	return in->keys[key].down;
}
