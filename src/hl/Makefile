include ../Makefile.inc

LDFLAG 	+=	-lIL -lILU -lm

HL_OBJECTS = 	hlColorSpace.o \
		hlColor.o \
		hlTile.o \
		hlRaw.o \
		hlImg.o \
		hlFrame.o \
		hlAdjOp.o \
		hlRegion.o \
		hlADT.o \
		hlOp.o \
		hlBlendOp.o \
		hlDrawOp.o \
		hlInit.o 


all: Makefile ../Makefile.inc test libhl
#	LIBRARY COMPILATION

libhl:	${HL_OBJECTS}
	${AR} ${ARFLAG} r libhl.a ${HL_OBJECTS}
	${RANLIB} libhl.a

#	BINARY FILES COMPILATION

test: ${HL_OBJECTS} hlTest.o
	${LD} ${LDFLAG} -o test.bin hlTest.o ${HL_OBJECTS}

#	INDIVIDUAL OBJECTS COMPILATIONS

CORE_H = hlColorSpace.h hlColor.h hlTile.h hlDataType.h

hlColorSpace.o: hlColorSpace.c hlColorSpace.h
	${CC} ${CCFLAG} -c hlColorSpace.c

hlColor.o: hlColor.c hlColor.h 
	${CC} ${CCFLAG} -c hlColor.c

hlTile.o: hlTile.c hlTile.h hlColorSpace.h hlColor.h
	${CC} ${CCFLAG} -c hlTile.c

hlRaw.o: hlRaw.c hlRaw.h hlColorSpace.h hlColor.h
	${CC} ${CCFLAG} -c hlRaw.c

hlImg.o: hlImg.c hlImg.h ${CORE_H}
	${CC} ${CCFLAG} -c hlImg.c

hlFrame.o: hlFrame.c hlFrame.h ${CORE_H}
	${CC} ${CCFLAG} -c hlFrame.c 

hlAdjOp.o: hlAdjOp.c hlAdjOp.h hlOp.h ${CORE_H}
	${CC} ${CCFLAG} -c hlAdjOp.c

hlRegion.o: hlRegion.c hlRegion.h
	${CC} ${CCFLAG} -c hlRegion.c

hlADT.o: hlADT.c hlADT.h
	${CC} ${CCFLAG} -c hlADT.c

hlOp.o: hlOp.c hlOp.h ${CORE_H}
	${CC} ${CCFLAG} -c hlOp.c

hlBlendOp.o: hlBlendOp.c hlBlendOp.h ${CORE_H}
	${CC} ${CCFLAG} -c hlBlendOp.c

hlDrawOp.o: hlDrawOp.c hlDrawOp.h ${CORE_H}
	${CC} ${CCFLAG} -c hlDrawOp.c

hlInit.o: hlInit.c ${CORE_H}
	${CC} ${CCFLAG} -c hlInit.c

#	CLEANING TARGET

clean:
	${ECHO} Cleaning 'hl'
	-${RM} *.o *.a test.bin

# 	DOCUMENTATION

doc: Doxygen.config *.h *.c
	doxygen Doxygen.config

