#include <stdlib.h>
#include <stdio.h>
#include <glib-object.h>
#include <cogl/cogl.h>
#include"uiButton.h"

G_DEFINE_TYPE(ClutterButton,clutter_button,CLUTTER_TYPE_GROUP);

enum{
	PROP_0,
	PROP_TEXTURE,
	PROP_ID
};

#define CLUTTER_BUTTON_GET_PRIVATE(obj)\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),CLUTTER_TYPE_BUTTON,ClutterButtonPrivate))

struct _ClutterButtonPrivate{
	ClutterTexture *texture;
	gint id;
};

static void clutter_button_paint(ClutterActor *self){
	ClutterButtonPrivate *priv = CLUTTER_BUTTON(self)->priv;
	cogl_push_matrix ();
	if(priv->texture){
		clutter_actor_paint(CLUTTER_ACTOR(priv->texture));
	}else{
		g_print("ERROR: no texture in button\n");
	}
}
static void clutter_button_pick(ClutterActor *self, const ClutterColor *color){
	CLUTTER_ACTOR_CLASS (clutter_button_parent_class)->pick (self, color);
}
static void clutter_button_dispose(GObject *object){
	ClutterButtonPrivate *priv = CLUTTER_BUTTON(object)->priv;
	if(priv->texture){
		clutter_actor_unparent(CLUTTER_ACTOR(priv->texture));
		priv->texture = NULL;
	}
  	G_OBJECT_CLASS (clutter_button_parent_class)->dispose (object);
}
static void clutter_button_finalize(GObject *object){
  	G_OBJECT_CLASS (clutter_button_parent_class)->finalize (object);
}
static void clutter_button_set_property(GObject *object,
					guint prop_id,
					const GValue *value,
					GParamSpec *pspec){
	ClutterButton *button = CLUTTER_BUTTON(object);
	switch(prop_id){
		case PROP_TEXTURE:
		clutter_button_set_texture(button,
					g_value_get_object(value));
		break;
		case PROP_ID:
		clutter_button_set_id(button,g_value_get_int(value));
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}
static void clutter_button_get_property(GObject *object,
					guint prop_id,
					GValue *value,
					GParamSpec *pspec){
	ClutterButtonPrivate *priv = CLUTTER_BUTTON(object)->priv;
	switch(prop_id){
		case PROP_TEXTURE:
		g_value_set_object(value,priv->texture);
		break;
		case PROP_ID:
		g_value_set_int(value,priv->id);
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}


static void clutter_button_class_init(ClutterButtonClass *klass){
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS(klass);

	actor_class->paint = clutter_button_paint;
	actor_class->pick  = clutter_button_pick;

	gobject_class->finalize = clutter_button_finalize;
	gobject_class->dispose  = clutter_button_dispose;
	gobject_class->set_property = clutter_button_set_property;
	gobject_class->get_property = clutter_button_get_property;
	
	g_type_class_add_private(gobject_class,sizeof(ClutterButtonPrivate));

	g_object_class_install_property(gobject_class,
			PROP_TEXTURE,
			g_param_spec_object("texture",
					"Texture",
					"The Background Texture of the Button",
					CLUTTER_TYPE_ACTOR,
					G_PARAM_READABLE | G_PARAM_WRITABLE));	
}
static void clutter_button_init(ClutterButton *self){
	self->priv = CLUTTER_BUTTON_GET_PRIVATE(self);
}
void clutter_button_set_texture(ClutterButton *button, ClutterTexture *texture){
	ClutterButtonPrivate *priv;
	g_return_if_fail(CLUTTER_IS_BUTTON(button));
  	g_return_if_fail(CLUTTER_IS_TEXTURE(texture));
	priv = button->priv;
	if(priv->texture == texture){
		return;
	}else if(priv->texture){
		clutter_actor_unparent(CLUTTER_ACTOR(texture));
		priv->texture = NULL;
	}
	if (texture){
      		ClutterActor *parent = clutter_actor_get_parent(CLUTTER_ACTOR(texture));
		if (G_UNLIKELY (parent != NULL)){
          		g_warning ("Unable to set the background texture of type `%s' for "
                     		"the frame of type `%s': the texture actor is already "
                     		"a child of a container of type `%s'",
                     		g_type_name (G_OBJECT_TYPE (texture)),
                     		g_type_name (G_OBJECT_TYPE (button)),
                    		g_type_name (G_OBJECT_TYPE (parent)));
          		return;
        	}	
		priv->texture = texture;
      		/*clutter_actor_set_parent(  CLUTTER_ACTOR(texture), 
					   CLUTTER_ACTOR(button)     );*/
		clutter_container_add_actor(CLUTTER_CONTAINER(button),
					    CLUTTER_ACTOR(texture));
		//clutter_actor_show(CLUTTER_ACTOR(texture));
		//clutter_actor_set_position(CLUTTER_ACTOR(texture),0,0);
    	}
	clutter_actor_queue_relayout (CLUTTER_ACTOR (button));
	g_object_notify (G_OBJECT (button), "texture");
}
void clutter_button_set_id(ClutterButton *button, gint id){
	g_return_if_fail(CLUTTER_IS_BUTTON(button));
	button->priv->id = id;
}
gint clutter_button_get_id(ClutterButton *button){
	if(!CLUTTER_IS_BUTTON(button)){
		g_print("WARNING : calling clutter_button_get_id() on not button widget\n");
		return 0;
	}
	return button->priv->id;
}
static gboolean button_enter(ClutterActor *a, ClutterEvent *e,gpointer data){
	ClutterEffectTemplate *et = clutter_effect_template_new_for_duration(150,CLUTTER_ALPHA_SINE_INC);
	clutter_effect_scale(et,a,1.10,1.10,NULL,NULL);
	return TRUE;
}
static gboolean button_leave(ClutterActor *a, ClutterEvent *e,gpointer data){
	ClutterEffectTemplate *et = clutter_effect_template_new_for_duration(250,CLUTTER_ALPHA_SINE_INC);
	clutter_effect_scale(et,a,1.0,1.0,NULL,NULL);
	return TRUE;
}
ClutterActor *clutter_button_new_at_pos(char *iconpath,gint x, gint y){
	guint sx, sy;
	ClutterActor *texture = clutter_texture_new_from_file(iconpath,NULL);
	ClutterActor *b =  g_object_new (CLUTTER_TYPE_BUTTON,
		       "texture",CLUTTER_TEXTURE(texture), NULL);
	clutter_actor_set_reactive(b,TRUE);
	clutter_actor_get_size(b,&sx,&sy);
  	clutter_actor_set_position (b,x+sx/2,y+sy/2);
	clutter_actor_set_anchor_point(b,sx/2,sy/2); 
  	g_signal_connect (b, "enter-event",G_CALLBACK (button_enter), NULL);
  	g_signal_connect (b, "leave-event",G_CALLBACK (button_leave), NULL);
	return b;
}
int main(int argc, char **argv){
	ClutterActor *stage;	
  	ClutterColor stage_color = { 80, 80, 80, 255 }; 
	
	clutter_init (&argc, &argv);
	
	stage = clutter_stage_get_default ();
  	clutter_actor_set_size (stage, 800, 600);
  	clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
	
	ClutterActor *button = clutter_button_new_at_pos("buttons/but+.png",30,30);
	clutter_container_add_actor (CLUTTER_CONTAINER (stage),
				     button);

	clutter_button_set_id(CLUTTER_BUTTON(button),42);
	g_print("%d\n",clutter_button_get_id(CLUTTER_BUTTON(button)));
	clutter_actor_show(button);
  	clutter_actor_show (stage);
	clutter_main();
	return EXIT_SUCCESS;
}
