include ../Makefile.inc

GEN_EXE = time_wait_test_server time_wait_test_client socketpipe sendfile_test \
	ephemeral_proof is_shell_cl is_shell_sv pftp_server pftp_client

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

sendfile_test: sendfile.o rdwrn.o

sendfile.o: sendfile.h

rdwrn.o: rdwrn.h

is_shell_cl: rdwrn.o

is_shell_sv: rdwrn.o

pftp_server: pftp_common.o

pftp_client: pftp_common.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation