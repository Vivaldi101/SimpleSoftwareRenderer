#include "keys.h"
Key	global_keys[MAX_KEYS];
b32		global_any_key_down;

void IN_ClearKeyStates() {
	//global_any_key_down = qfalse;

	for (int i = 0; i < MAX_KEYS; ++i) {
		if (global_keys[i].down) {
			//CL_KeyEvent(i, qfalse, 0);
		}
		global_keys[i].down = 0;
		global_keys[i].repeats = 0;
	}
}

void IN_HandleKeyEvent(int key, b32 down, unsigned time) {
	char	*kb;
	char	cmd[1024];

	// update auto-repeat status and BUTTON_ANY status
	global_keys[key].down = down;

	if (down) {
		global_keys[key].repeats++;
		if ( global_keys[key].repeats == 1) {
			global_any_key_down++;
		}
	} else {
		global_keys[key].repeats = 0;
		global_any_key_down--;
		if (global_any_key_down < 0) {
			global_any_key_down = 0;
		}
	}

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

#ifdef __linux__
  if (key == K_ENTER)
  {
    if (down)
    {
      if (keys[K_ALT].down)
      {
        Key_ClearStates();
        if (Cvar_VariableValue("r_fullscreen") == 0)
        {
          Com_Printf("Switching to fullscreen rendering\n");
          Cvar_Set("r_fullscreen", "1");
        }
        else
        {
          Com_Printf("Switching to windowed rendering\n");
          Cvar_Set("r_fullscreen", "0");
        }
        Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n");
        return;
      }
    }
  }
#endif

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
