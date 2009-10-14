include Makefile.inc

LDFLAG 	+= 	`sdl-config --libs` \
		`pkg-config --libs cairo` \
		-lGL -lGLU -lm 

LDFLAG 	+=	-Lgui -lgui

LDFLAG 	+=	-Lhl -lhl

EXE 	= 	main.bin


all: Makefile Makefile.inc main

main: ${UI_OBJECTS} uiMain.o
	${ECHO} Compile src:	${MAKE} ${MFLAG}
	${LD} -o ${EXE} uiMain.o ${LDFLAG}

uiMain.o:  uiMain.c gui/uiGui.h libgui.a hl/hlImg.h libhl.a
	${CC} ${CCFLAG} -c uiMain.c

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


