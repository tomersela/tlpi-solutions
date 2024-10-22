include ../Makefile.inc

EXE = free_and_sbrk_modified mymalloc

all : ${EXE}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
