include ../Makefile.inc

GEN_EXE = t_stat t_stat_mod estat chmod_r simple_chattr
GEN_DEP = file_perms.c
GEN_OBJS = $(GEN_DEP:.c=.o)

LINUX_EXE = 

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${GEN_OBJS} ${TLPI_LIB}		# True as a rough approximation
