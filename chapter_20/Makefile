include ../Makefile.inc

GEN_EXE = sig_receiver sig_receiver_mod sig_ign sh_nodefer sa_resethand siginterrupt
GEN_DEP = signal_functions.c
GEN_OBJS = $(GEN_DEP:.c=.o)

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${GEN_OBJS} ${TLPI_LIB}		# True as a rough approximation
