include Makefile.inc

LDFLAG 	+= 	`sdl-config --libs` \
		`pkg-config --libs cairo` \
		-lGL -lGLU -lm \
		-lIL -lILU

LDFLAG 	+=	-Lgui -lgui

LDFLAG 	+=	-Lhl -lhl

EXE 	= 	main.bin

MAIN_OBJECTS = 	uiMain.o uiHl.o
all: Makefile Makefile.inc main

main: ${MAIN_OBJECTS}
	${ECHO} Compile src:	${MAKE} ${MFLAG}
	${LD} -o ${EXE} ${MAIN_OBJECTS} ${LDFLAG}

uiMain.o:  uiMain.c libgui.a libhl.a
	${CC} ${CCFLAG} -c uiMain.c

uiHl.o: uiHl.c uiHl.h libgui.a libhl.a
	${CC} ${CCFLAG} -c uiHl.c

libgui.a: force_look
	${ECHO} Compile libgui:	${MAKE} ${MFLAG}
	cd gui; ${MAKE} ${MFLAG};

libhl.a: force_look
	${ECHO} Compile libhl:	${MAKE} ${MFLAG}
	cd hl; ${MAKE} ${MFLAG};

force_look:
	true

clean:
	${ECHO} Cleaning 'src'
	-${RM} main.bin  *.o
	cd gui;   ${MAKE} ${MFLAG} clean;
	cd hl; ${MAKE} ${MFLAG} clean;


