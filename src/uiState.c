#include "uiState.h"
#define ASCII_KEY_COUNT 256
static char key_state[ASCII_KEY_COUNT];
static char state[UI_STATE_COUNT];
static char mod_key_state[UI_MOD_KEYS_COUNT];

#define UI_KEY_DOWN 1
#define UI_KEY_UP   0

void  set_key_status(guint key, int status){
	if(key < ASCII_KEY_COUNT){
		key_state[key] = status;
	}else{
		switch(key){
			case GDK_Escape:
			mod_key_state[UI_ESC] = status;
			break;
			case GDK_Tab:
			mod_key_state[UI_TAB] = status;
			break;
			case GDK_Enter:
			mod_key_state[UI_] = status;
			break;
			case GDK_Left:
			mod_key_state[UI_LEFT] = status;
			break;
			case GDK_Up:
			mod_key_state[UI_UP] = status;
			break;
			case GDK_Right:
			mod_key_state[UI_RIGHT] = status;
			break;
			case GDK_Down:
			mod_key_state[UI_DOWN] = status;
			break;
			case GDK_Shift_L
			mod_key_state[UI_SHIFT_L] = status;
			if(status == UI_KEY_DOWN){
				mod_key_state[UI_SHIFT] = status;
			}else{
				if (mod_key_state[UI_SHIFT_R] == UI_KEY_DOWN){
					mod_key_state[UI_SHIFT] = UI_KEY_DOWN;
				}else{
					mod_key_state[UI_SHIFT] = UI_KEY_UP;
				}
			}
			break;
			case GDK_Shift_R
			mod_key_state[UI_SHIFT_R] = status;
			if(status == UI_KEY_DOWN){
				mod_key_state[UI_SHIFT] = status;
			}else{
				if (mod_key_state[UI_SHIFT_L] == UI_KEY_DOWN){
					mod_key_state[UI_SHIFT] = UI_KEY_DOWN;
				}else{
					mod_key_state[UI_SHIFT] = UI_KEY_UP;
				}
			}
			break;
			case GDK_Control_L
			mod_key_state[UI_CTRL_L] = status;
			if(status == UI_KEY_DOWN){
				mod_key_state[UI_CTRL] = status;
			}else{
				if (mod_key_state[UI_CTRL_R] == UI_KEY_DOWN){
					mod_key_state[UI_CTRL] = UI_KEY_DOWN;
				}else{
					mod_key_state[UI_CTRL] = UI_KEY_UP;
				}
			}
			break;
			case GDK_Control_R
			mod_key_state[UI_CTRL_R] = status;
			if(status == UI_KEY_DOWN){
				mod_key_state[UI_CTRL] = status;
			}else{
				if (mod_key_state[UI_CTRL_L] == UI_KEY_DOWN){
					mod_key_state[UI_CTRL] = UI_KEY_DOWN;
				}else{
					mod_key_state[UI_CTRL] = UI_KEY_UP;
				}
			}
			break;
			case GDK_Alt_L
			mod_key_state[UI_ALT_L] = status;
			if(status == UI_KEY_DOWN){
				mod_key_state[UI_ALT] = status;
			}else{
				if (mod_key_state[UI_ALT_R] == UI_KEY_DOWN){
					mod_key_state[UI_ALT] = UI_KEY_DOWN;
				}else{
					mod_key_state[UI_ALT] = UI_KEY_UP;
				}
			}
			break;
			case GDK_Alt_R
			mod_key_state[UI_ALT_R] = status;
			if(status == UI_KEY_DOWN){
				mod_key_state[UI_ALT] = status;
			}else{
				if (mod_key_state[UI_ALT_L] == UI_KEY_DOWN){
					mod_key_state[UI_ALT] = UI_KEY_DOWN;
				}else{
					mod_key_state[UI_ALT] = UI_KEY_UP;
				}
			}
			break;
		}
	}
}
static gboolean key_event_press(ClutterActor *a, ClutterEvent *e, gpointer data){
	guint key = clutter_key_event_symbol(&(e->key));
	printf("key:%d\n",keys);
	set_key_status(key,UI_KEY_DOWN);	
	return FALSE; 	/*don't block the event there.*/
}
static gboolean key_event_release(ClutterActor *a, ClutterEvent *e, gpointer data){
	guint key = clutter_key_event_symbol(&(e->key));
	printf("key:%d\n",keys);
	set_key_status(key,UI_KEY_UP);	
	return FALSE; 	/*don't block the event there.*/
}
void set_mouse_status(ClutterEvent *e, int status){
	if (e->button.modifier_state & CLUTTER_BUTTON1_MASK){
		state[UI_MOUSE1_PRESSED] = status;
	}
	if (e->button.modifier_state & CLUTTER_BUTTON2_MASK){
		state[UI_MOUSE2_PRESSED] = status;
	}
	if (e->button.modifier_state & CLUTTER_BUTTON3_MASK){
		state[UI_MOUSE3_PRESSED] = status;
	}
}
static gboolean mouse_event_press(ClutterActor *a, ClutterEvent *e, gpointer data){
	set_mouse_status(e,UI_KEY_DOWN);
}
static gboolean mouse_event_release(ClutterActor *a, ClutterEvent *e, gpointer data){
	set_mouse_status(e,UI_KEY_UP);
}
int uiStateSet(enumui_state s, char value){
	int old = state[s];
	state[s] = value;
	return old;
}
int uiStateGet(enum ui_state uis){
	return state[uis];
}
int uiStateKey(char key){
	return key_state[key];
}
int uiStateModKey(enum ui_mod_keys modkey){
	return mod_key_state[modkey];
}

