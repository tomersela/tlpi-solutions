include ../Makefile.inc

GEN_EXE = svsem_op fork_sig_sync fork_sig_sync_mod svsem_create svsem_mon svsem_setall \
			conditional_reserve vms_flags_test

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

# Helper library
binary_sems.o : binary_sems.c
binary_sems_mod.o : binary_sems_mod.c
vms_flags.o : vms_flags.c

# Link all bandwidth programs with the following libs
${GEN_EXE} : binary_sems_mod.o vms_flags.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
