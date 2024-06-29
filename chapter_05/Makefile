include ../Makefile.inc

EXE = large_file append_beginning atomic_append dedup dup_stat_verifier

all : ${EXE}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
