include ../Makefile.inc

GEN_EXE = uppercase popen fifo_seqnum_server fifo_seqnum_client \
		fifo_seqnum_server_mod fifo_seqnum_server_mod2 \
		fifo_seqnum_server_mod3 bad_client fifo_seqnum_client_mod \
		nonblocking_open_reader nonblocking_open_writer nonblocking_io

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
