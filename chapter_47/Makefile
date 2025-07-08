include ../Makefile.inc

GEN_EXE = svsem_op fork_sig_sync fork_sig_sync_mod svsem_create svsem_mon svsem_setall

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

# Helper library
binary_sems.o : binary_sems.c

# Link all bandwidth programs with binary_sems.o
${GEN_EXE} : binary_sems.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
