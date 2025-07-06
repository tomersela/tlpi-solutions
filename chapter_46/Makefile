include ../Makefile.inc

GEN_EXE = svmsg_create svmsg_send svmsg_receive msg_seqnum_client msg_seqnum_server \
	svmsg_file_client svmsg_file_server svmsg_file_client_mod svmsg_file_server_mod_a \
	svmsg_file_server_mod_b svmsg_file_server_mod_c svmsg_file_server_mod_d \
	svmsg_file_server_mod_e svmsg_bad_file_client

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
