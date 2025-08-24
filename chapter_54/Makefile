include ../Makefile.inc

GEN_EXE = svshm_xfr_reader svshm_xfr_writer pshm_xfr_reader pshm_xfr_writer

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

binary_sems.o: semun.h

svshm_xfr_reader svshm_xfr_writer: binary_sems.o

pshm_xfr_reader pshm_xfr_writer: binary_sems.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
