include ../Makefile.inc

GEN_EXE = poll_pipes select_pipes id_echo_cl id_echo_sv is_echo_cl echo_server \
	msgq_select select_self_pipe epoll_wait_empty epoll_fairness demo_sigio demo_rtsigio
	

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

inet_sockets.o: inet_sockets.h
become_daemon.o: become_daemon.h
tty_functions.o: tty_functions.h

id_echo_cl.o id_echo_sv.o echo_server.o: id_echo.h
id_echo_cl id_echo_sv is_echo_cl echo_server: inet_sockets.o become_daemon.o
demo_sigio demo_rtsigio: tty_functions.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
