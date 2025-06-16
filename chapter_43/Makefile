include ../Makefile.inc

GEN_EXE = pipe_bandwidth posix_msgq_bandwidth unix_stream_bandwidth uds_bandwidth sysv_msgq_bandwidth

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

# Helper library
helper.o : helper.c helper.h

# Link all bandwidth programs with helper.o
${GEN_EXE} : helper.o


clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
