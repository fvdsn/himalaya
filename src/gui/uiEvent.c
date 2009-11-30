#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "uiCore.h"

#define UI_MAX_EVENT 1000
#define UI_MAX_LISTENER 1000
/*
static uiEntity *listener[UI_MAX_LISTENER];
static int listener_channel[UI_MAX_LISTENER];
static int listener_type[UI_MAX_LISTENER];
static int event_count;
static uiEvent event[UI_MAX_EVENT];
static int callback_count;
static int (*callback[UI_MAX_LISTENER])(uiEvent *e);

void uiEventSend(uiEntity *src, int channel, int type, int datasize, void *data){
	uiEvent * e = event + event_count;
	if(event_count >= UI_MAX_EVENT){
		printf("ERROR: too much events\n");
		return;
	}
	e->type = type;
	e->source = src;
	e->channel = channel;
	e->datasize = datasize;
	e->data = data;
	event_count++;
}
void uiEventListen(uiEntity *ent, int channel, int type ){
	int i = 0;
	while (i < UI_MAX_LISTENER){
		if (!listener[i]){
			listener[i] = ent;
			listener_channel[i] = channel;
			listener_type[i] = type;
			return;
		}
		i++;
	}
	printf("ERROR : too much listeners\n");
}
void uiEventForget(uiEntity *ent){
	int i = 0;
	while(i < UI_MAX_LISTENER){
		if(listener[i] == ent){
			listener[i] = NULL;
		}
		i++;
	}
}
void uiEventCallback(int channel, int type, int (*cb)(uiEvent *e)){
	if(callback_count >= UI_MAX_LISTENER){
		printf("ERROR: too much callback\n");
		return;
	}
	callback[callback_count] = cb;
	callback_count ++;
}
void uiEventProcess(void){
	int i = 0;
	int j = 0;
	while(i < event_count){
		j = 0;
		while(j < callback_count){
			if(callback[j]){
				callback[j](event+i);
			}
			j++;
		}
		j = 0;
		while (j < UI_MAX_LISTENER){
			if(	listener[j]
				&& listener[j]->event
				&& event[i].type == listener_type[j]
				&& event[i].channel == listener_channel[j] ){
				listener[j]->event(listener[j],event + i);
			}
			j++;
		}
		i++;
	}
	/x*TODO free vent data *x/
	event_count = 0;
}
*/				
#define UI_MAX_ACTION 256
static uiAction action[UI_MAX_ACTION];
static int action_count = 0;

void uiActionRegister(const char *name,
		int entity_type,
		const char *state_name,
		int state_value,
		int modkey,
		char hotkey,
		int  mouse_button,
		int (*action_start)(uiAction *self, uiEntity *active), 
		int (*action_hold)(uiAction *self, uiEntity *active),
		int (*action_end)(uiAction *self, uiEntity *active) ){
	if(action_count >= UI_MAX_ACTION){
		printf("WARNING : failed to register action, max action reached \n");
	}
	strncpy(action[action_count].name,name,UI_NAME_LENGTH); 
	/*TODO put '\0' in string if state_name is null */
	strncpy(action[action_count].state_name,state_name,UI_NAME_LENGTH); 
	action[action_count].entity_type = entity_type;
	action[action_count].state_value = state_value;
	action[action_count].hotkey = hotkey;
	action[action_count].modkey = modkey;
	action[action_count].mouse_button = mouse_button;
	action[action_count].action_start = action_start; /*TODO hold, end */
}
void uiActionProcess(void){
	int i = action_count;
	int process = 1;
	
	/*process actions not related to any entity*/
	while(i--){
		process = 1;
		if(	action[i].entity_type != UI_ENT_ANY ){
			continue;
		}
		if(	action[i].hotkey != UI_KEY_ANY 
				&& uiStateKey(action[i].hotkey) != UI_KEY_DOWN){
			continue;
		}
		/*TODO MOD_ANY won't work with zero mod pressed */
		if( uiStateMod(action[i].modkey) != UI_KEY_DOWN ){
			continue;
		}
		if( action[i].mouse_button != UI_MOUSE_BUTTON_ANY &&
				uiStateMouse(action[i].mouse_button) != UI_KEY_DOWN){
			continue;
		}
		/*
		if(uiStateKeyStatus(action[i].ho
		*/
	}
	
}
