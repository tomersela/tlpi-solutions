include ../Makefile.inc

GEN_EXE = poll_pipes select_pipes id_echo_cl id_echo_sv is_echo_cl echo_server

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

inet_sockets.o: inet_sockets.h
become_daemon.o: become_daemon.h

id_echo_cl.o id_echo_sv.o echo_server.o: id_echo.h
id_echo_cl id_echo_sv is_echo_cl echo_server: inet_sockets.o become_daemon.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
