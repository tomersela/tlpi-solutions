include ../Makefile.inc

GEN_EXE = t_flock flock_deadlock flock_inheritance i_fcntl_locking lock_acquirer \
	lock_tester mandatory_deadlock_test simple_lockfile

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
