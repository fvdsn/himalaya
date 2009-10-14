include ../Makefile.inc

LDFLAG += 	`sdl-config --libs` \
		`pkg-config --libs cairo` \
		-lGL -lGLU -lm

CCFLAG +=	`sdl-config --cflags` \
		`pkg-config --cflags cairo`

UI_OBJECTS = uiState.o uiEvent.o uiCore.o uiIterator.o uiWidget.o uiTabWidget.o uiEntity.o uiString.o uiDraw.o

all: Makefile ../Makefile.inc test main libgui
#	LIBRARY COMPILATION

libgui:	${UI_OBJECTS}
	${AR} ${ARFLAG} r libgui.a ${UI_OBJECTS}
	${RANLIB} libgui.a

#	BINARY FILES COMPILATION

main: ${UI_OBJECTS} uiMain.o
	${LD} ${LDFLAG} -o main.bin uiMain.o ${UI_OBJECTS}

test: ${UI_OBJECTS} uiTest.o
	${LD} ${LDFLAG} -o test.bin uiTest.o ${UI_OBJECTS}

#	INDIVIDUAL OBJECTS COMPILATIONS

uiState.o: uiState.c uiCore.h
	${CC} ${CCFLAG} -c uiState.c

uiEvent.o: uiEvent.c uiCore.h
	${CC} ${CCFLAG} -c uiEvent.c

uiCore.o: uiCore.c uiCore.h
	${CC} ${CCFLAG} -c uiCore.c

uiIterator.o: uiIterator.c uiIterator.h
	${CC} ${CCFLAG} -c uiIterator.c

uiWidget.o: uiWidget.c uiWidget.h uiCore.h
	${CC} ${CCFLAG} -c uiWidget.c

uiTabWidget.o: uiTabWidget.c uiWidget.h uiCore.h
	${CC} ${CCFLAG} -c uiTabWidget.c

uiEntity.o: uiEntity.c uiCore.h uiIterator.h
	${CC} ${CCFLAG} -c uiEntity.c

uiString.o: uiString.c uiCore.h
	${CC} ${CCFLAG} -c uiString.c

uiDraw.o: uiDraw.c uiCore.h
	${CC} ${CCFLAG} -c uiDraw.c

uiTest.o: uiTest.c uiCore.h uiWidget.h
	${CC} ${CCFLAG} -c uiTest.c

uiMain.o: uiMain.c uiCore.h uiWidget.h
	${CC} ${CCF:AG} -c uiMain.c

#	CLEANING TARGET

clean:
	${ECHO} Cleaning 'gui'
	-${RM} *.o *.a main.bin test.bin

# 	DOCUMENTATION

doc: Doxygen.config *.h *.c
	doxygen Doxygen.config

