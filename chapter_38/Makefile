include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = douser

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}


douser : douser.o
	${CC} -o $@ douser.o ${LDFLAGS} ${IMPL_LDLIBS} ${LINUX_LIBCRYPT}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
