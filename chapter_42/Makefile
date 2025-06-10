include ../Makefile.inc

GEN_EXE =

LINUX_EXE = close_lib dynload dynload_mod

# Shared libraries for Linux
SHARED_LIBS = liba.so libb.so

EXE = ${GEN_EXE} ${LINUX_EXE}
LIBS = ${SHARED_LIBS}

all : ${EXE} ${LIBS}

allgen : ${GEN_EXE}

clean :
	${RM} ${EXE} ${LIBS} dylib.o *.o

showall :
	@ echo ${EXE} ${LIBS}

${EXE} : ${TLPI_LIB} dylib.o		# Link with dylib.o

# Rules for shared libraries
liba.so: liba.c dylib.o
	${CC} ${CFLAGS} -fPIC -shared -o $@ $< dylib.o

libb.so: libb.c dylib.o
	${CC} ${CFLAGS} -fPIC -shared -o $@ $< dylib.o

# Static object file for dylib
dylib.o: dylib.c
	${CC} ${CFLAGS} -c -o $@ $<
