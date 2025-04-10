include ../Makefile.inc

GEN_EXE = signal_threads

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}


signal_threads : signal_threads.o signal_functions.o ${TLPI_LIB}
	${CC} ${CFLAGS} signal_threads.o signal_functions.o -o signal_threads ${LDLIBS}

signal_threads.o : signal_threads.c signal_functions.h

signal_functions.o : signal_functions.c signal_functions.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
