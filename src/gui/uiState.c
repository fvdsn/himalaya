#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uiCore.h"

#define UI_MAX_STATE_COUNT 1000

static int key_state[256];
static int mod_key_state[UI_MOD_KEY_COUNT];
static int mouse_button_state[UI_MOD_KEY_COUNT];
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
void uiStateSetKey(char key, int value){
	key_state[(int)key] = value;
}
void uiStateSetMod(int mod, int value){
	mod_key_state[mod] = value;
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
int  uiStateKey(char key){
	return key_state[(int)key];
}
int  uiStateMod(int mod){
	return mod_key_state[mod];
}
int  uiStateMouse(int button){
	return mouse_button_state[button];
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

