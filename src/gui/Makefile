APP		=	gui-test

CC		=	gcc

CFLAGS	=	-Wall -c -g -std=c99 \
			`sdl-config --cflags` \
			`pkg-config --cflags cairo` \
			`freetype-config --cflags`

LDFLAGS	=	`sdl-config --libs` \
			`pkg-config --libs cairo` \
			-lGL -lGLU -lm

SRCS	=	uiCore.c \
			uiEntity.c \
			uiEvent.c \
			uiDraw.c \
			uiWidget.c \
			uiTest.c \
			uiState.c \
			uiFont.c \
			uiIterator.c

OBJS	=	$(SRCS:.c=.o)

all: $(APP)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# it is important that $(OBJS) stands _before_ $(LDFLAGS)
$(APP):	$(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o$(APP)

clean:
	rm -f *.o  *~ $(APP) 

