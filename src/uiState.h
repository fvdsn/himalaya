#ifndef __UI_STATE_H__
#define __UI_STATE_H__
#include <clutter/clutter.h>

enum ui_state{
	UI_MOUSE_1_PRESSED,
	UI_MOUSE_2_PRESSED,
	UI_MOUSE_3_PRESSED,
	UI_ON_DESK,
	UI_ON_CANVAS,
	UI_ON_TOOLBAR,
	UI_ON_BUTTON,
	UI_TOOL_PAINT,
	UI_TOOL_MOVE,
	UI_TOOL_PAINTING,
	UI_TOOL_PAINTING_OFFCANVAS,
	UI_STATE_COUNT
};
enum ui_mod_keys{
	UI_CTRL,
	UI_CTRL_L,
	UI_CTRL_R,
	UI_ALT,
	UI_ALT_L,
	UI_ALT_R,
	UI_ESC,
	UI_SPACE,
	UI_SHIFT,
	UI_SHIFT_L,
	UI_SHIFT_R,
	UI_ENTER,
	UI_LEFT,
	UI_RIGHT,
	UI_UP,
	UI_DOWN,
	UI_TAB,
	UI_MOD_KEYS_COUNT
};
void uiStateInit(ClutterStage *stage);

int uiStateGet(enum ui_state uis);
int uiStateSet(enum ui_state s, char value);
int uiStateKey(char key);
int uiStateModKey(enum ui_mod_keys modkey);

#endif
