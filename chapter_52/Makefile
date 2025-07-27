include ../Makefile.inc

GEN_EXE = pmsg_receive pmsg_create pmsg_send pmsg_receive_to \
			fifo_seqnum_client fifo_seqnum_server pmq_seqnum_client pmq_seqnum_server \
			svmsg_file_client svmsg_file_server pmq_file_client pmq_file_server \
			chat mq_notify_sig mq_notify_sig_mod

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

fifo_seqnum_client.o fifo_seqnum_server.o : fifo_seqnum.h
pmq_seqnum_client.o pmq_seqnum_server.o : pmq_seqnum.h

svmsg_file_client.o svmsg_file_server.o : svmsg_file.h
pmq_file_client.o pmq_file_server.o : pmq_file.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
