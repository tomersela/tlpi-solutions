include ../Makefile.inc

GEN_EXE = svsem_op fork_sig_sync fork_sig_sync_mod svsem_create svsem_mon svsem_setall \
			conditional_reserve vms_flags_test npipe_sem_test_simple npipe_sem_test svsem_ls

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

vms_flags_test: vms_flags.o
npipe_sem_test: npipe_sem.o
npipe_sem_test_simple: npipe_sem.o

# Helper libraries
binary_sems_mod.o : binary_sems_mod.c
vms_flags.o : vms_flags.c
npipe_sem.o: npipe_sem.c


# Link all bandwidth programs with the following libs
${GEN_EXE} : binary_sems_mod.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
