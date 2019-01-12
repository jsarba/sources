# Makefile

CC=gcc
RM=rm

CFLAGS= -O -Wall #-g #use -g debug mode when debug is needed *only*
LIBS= -Llibs
INCLUDES=-Iincludes

OBJS= static_api.o entry-2.o misc.o pe_method.o \
		t_core.o dll_method.o patch_iat.o total.o
DLL=lib-krnl.o

DLLNAME=lib-krnl.dll
PROGRAM=sanguijuela.exe #temporal

all: ${OBJS} ${DLL}
	${CC} ${CFLAGS} ${OBJS} -o ${PROGRAM}
	${CC} ${CFLAGS} -shared ${DLL} -o ${DLLNAME}
	
total.o: total.c misc.h
	${CC} ${CFLAGS} -c total.c

static_api.o: static_api.s
	${CC} ${CFLAGS} -c static_api.s

entry-2.o: entry-2.s
	${CC} ${CFLAGS} -c entry-2.s

misc.o: misc.c
	${CC} ${CFLAGS} -c misc.c

t_core.o: t_core.c misc.h
	${CC} ${CFLAGS} -c t_core.c

pe_method.o: pe_method.c
	${CC} ${CFLAGS} -c pe_method.c

dll_method.o: dll_method.c
	${CC} ${CFLAGS} -c dll_method.c

patch_iat.o: patch_iat.c
	${CC} ${CFLAGS} -c patch_iat.c

lib-krnl.o: lib-krnl.c
	${CC} ${CFLAGS} -c lib-krnl.c

clean:
	${RM} -f ${OBJS} ${DLL}

clean-all:
	${RM} -f ${OBJS} ${DLL} ${PROGRAM} ${DLLNAME}
