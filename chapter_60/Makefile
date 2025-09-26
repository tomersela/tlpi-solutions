include ../Makefile.inc

GEN_EXE = is_echo_sv is_echo_sv_mod is_echo_sv_mod2

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}


is_echo_sv: inet_sockets.o become_daemon.o
is_echo_sv_mod: inet_sockets.o become_daemon.o
is_echo_sv_mod2: inet_sockets.o become_daemon.o

inet_sockets.o: inet_sockets.h
become_daemon.o: become_daemon.h


clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
