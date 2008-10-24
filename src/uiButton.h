#ifndef __UI_BUTTON_H__
#define __UI_BUTTON_H__
#include <clutter/clutter-actor.h>
#include<clutter/clutter.h>

#define CLUTTER_TYPE_BUTTON clutter_button_get_type()
#define CLUTTER_BUTTON(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), CLUTTER_TYPE_BUTTON,ClutterButton))
#define CLUTTER_BUTTON_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), CLUTTER_TYPE_BUTTON,ClutterButtonClass))
#define CLUTTER_IS_BUTTON(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), CLUTTER_TYPE_BUTTON))
#define CLUTTER_IS_BUTTON_CLASS(klass)\
	(G_TYPE_CHECK_CLASS_TYPE((klass), CLUTTER_TYPE_BUTTON))
#define CLUTTER_BUTTON_GET_CLASS(obj)\
	(G_TYPE_INSTANCE_GET_CLASS((obj), CLUTTER_TYPE_BUTTON, ClutterButtonClass))

typedef struct _ClutterButton 		ClutterButton;
typedef struct _ClutterButtonClass 	ClutterButtonClass;
typedef struct _ClutterButtonPrivate 	ClutterButtonPrivate;

struct _ClutterButton{
	ClutterGroup parent_instance;
	ClutterButtonPrivate *priv;
};
struct _ClutterButtonClass{
	ClutterGroupClass parent_class;
};

GType clutter_button_get_type(void);
ClutterActor *clutter_button_new_at_pos(char *iconpath,gint x, gint y);
void clutter_button_set_texture(ClutterButton *button, ClutterTexture *texture);
void clutter_button_set_id(ClutterButton *button, gint id);
gint clutter_button_get_id(ClutterButton *button);

#endif
