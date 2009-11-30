#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uiCore.h"

#define UI_KEY_COUNT 256
#define UI_MAX_STATE_COUNT 1000
static int mod_key_state;
static int key_state[UI_KEY_COUNT];	/*the state in this frame */
static int key_state_p[UI_KEY_COUNT];	/*the state in the previous frame*/
static int key_state_h[UI_KEY_COUNT];	/*if this frame is a press, release or holding of a key*/
static int mouse_button_state[UI_MOUSE_BUTTON_COUNT];
static int mouse_button_state_p[UI_MOUSE_BUTTON_COUNT];
static int mouse_button_state_h[UI_MOUSE_BUTTON_COUNT];
static float mouse_x;
static float mouse_y;
static float mouse_pressure;
static float mouse_dx;
static float mouse_dy;
static float mouse_dpressure;
static int state_count = 0;
static uiState state[UI_MAX_STATE_COUNT];

static uiState* uiStateGet(const char *name){
	int i = state_count;
	while(i--){
		if(!strncmp(state[i].name,name,UI_NAME_LENGTH)){
			return state + i;
		}
	}
	i = state_count;
	state_count++;
	strncpy(state[i].name,name,UI_NAME_LENGTH);
	return state + i;
}
void uiStateProcess(){
	int i = UI_KEY_COUNT;
	/*setting keyboard state */
	while(i--){
		if ( key_state[i] == UI_KEY_DOWN && key_state_p[i] == UI_KEY_UP){
			key_state_h[i] = UI_KEY_PRESS;
			printf("Key '%d' PRESS\n",i);
		}else if ( key_state[i] == UI_KEY_UP && key_state_p[i] == UI_KEY_DOWN){
			key_state_h[i] = UI_KEY_RELEASE;
			printf("Key '%d' RELEASE\n",i);
		}else{
			if(key_state_h[i] != UI_KEY_HOLD){
				key_state_h[i] = UI_KEY_HOLD;
				printf("Key '%d' HOLD\n",i);
			}
		}
		key_state_p[i] = key_state[i];
	}
	/*setting mouse state */
	i = UI_MOUSE_BUTTON_COUNT;
	while(i--)
		if ( mouse_button_state[i] == UI_KEY_DOWN && mouse_button_state_p[i] == UI_KEY_UP){
			mouse_button_state_h[i] = UI_KEY_PRESS;
		}else if ( mouse_button_state[i] == UI_KEY_UP && mouse_button_state_p[i] == UI_KEY_DOWN){
			mouse_button_state_h[i] = UI_KEY_RELEASE;
		}else{
			mouse_button_state_h[i] = UI_KEY_HOLD;
		}
		mouse_button_state_p[i] = mouse_button_state[i];
	}
void uiStateSetValue(const char *name, int value){
	uiStateGet(name)->value = value;
}
void uiStateSetEnt(const char *name, uiEntity *ent){
	uiStateGet(name)->ent = ent;
}
int  uiStateGetValue(const char *name){
	return uiStateGet(name)->value;
}
uiEntity* uiStateGetEnt(const char *name){
	return uiStateGet(name)->ent;
}
void uiStateSetKey(int key, int value){
	if(key >= 0 && key < 256){
		key_state[key] = value;
	}
}
void uiStateSetMod(int mod, int value){
	if(value){
		printf("Mod '%d' ACTIVATED \n",mod); 
		mod_key_state |= mod;
	}else{
		printf("Mod '%d' CANCELLED \n",mod); 
		mod_key_state &= ~mod;
	}
}
void uiStateSetMouse(int button, int value){
	float x,y,p;
	uiStateMousePos(&x,&y,&p);
	if(value != 0){
		value = 1;
	}
	if(mouse_button_state[button] != value){
		mouse_button_state[button] = value;
		uiEventMouseButton(button,value,x,y,p);
	}
}
void uiStateSetMousePos(float x, float y, float pressure){
	if(mouse_x != x || mouse_y != y || mouse_pressure != pressure){
		mouse_dx = x - mouse_x;
		mouse_x = x;
		mouse_dy = y - mouse_y;
		mouse_y = y;
		mouse_dpressure = pressure - mouse_pressure;
		mouse_pressure = pressure;
		uiEventMouseMotion(x,y,pressure);
	}
}
int  uiStateKey(int key){
	if(key >= 0 && key < 256){
		return key_state[key];
	}else{
		printf("WARNING : could not get key state for key #%d\n",key);
		return UI_KEY_UP;
	}
}
int  uiStateMod(int mod){
	switch(mod){
		case UI_SHIFT:
			return uiStateMod(UI_SHIFT_L | UI_SHIFT_R);
		case UI_CTRL:
			return uiStateMod(UI_CTRL_L | UI_CTRL_R);
		case UI_ALT:
			return uiStateMod(UI_ALT_L | UI_ALT_R);
		default: 
			return mod_key_state & mod;
	}
}
int  uiStateMouse(int button){
	if(button >= 0 && button < UI_MOUSE_BUTTON_COUNT){
		return mouse_button_state[button];
	}else{
		printf("WARNING : could not get button state status for mouse button #%d\n",button);
		return UI_KEY_UP;
	}
}
int  uiStateKeyStatus(int key){
	if(key >= 0 && key < UI_KEY_COUNT){
		return key_state_h[key];
	}else{
		printf("WARNING : could not get key state status for key #%d\n",key);
		return UI_KEY_HOLD;
	}
}
int  uiStateMouseStatus(int button){
	if(button >= 0 && button < UI_MOUSE_BUTTON_COUNT){
		return mouse_button_state_h[button];
	}else{
		printf("WARNING : could not get key state status for mouse button #%d\n",button);
		return UI_KEY_HOLD;
	}
}
void  uiStateMousePos(float *x, float *y, float *pressure){
	if(x){
		*x = mouse_x;
	}
	if(y){
		*y = mouse_y;
	}
	if(pressure){
		*pressure = mouse_pressure;
	}
}
void  uiStateMouseDelta(float *x, float *y, float *pressure){
	if(x){
		*x = mouse_dx;
	}
	if(y){
		*y = mouse_dy;
	}
	if(pressure){
		*pressure = mouse_dpressure;
	}
}

