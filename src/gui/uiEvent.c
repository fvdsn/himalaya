#include<stdlib.h>
#include<stdio.h>
#include "uiCore.h"

#define UI_MAX_EVENT 1000
#define UI_MAX_LISTENER 1000

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
	/*TODO free vent data */
	event_count = 0;
}
				
