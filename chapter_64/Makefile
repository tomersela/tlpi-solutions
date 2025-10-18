include ../Makefile.inc

GEN_EXE = script script_mod_a script_mod_b script_mod_3 script_ts script_replay
	

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

pty_fork.o: pty_fork.h
tty_functions.o: tty_functions.h
pty_master_open.o: pty_master_open.h

script script_mod_a script_mod_b script_mod_3 script_ts: tty_functions.o pty_fork.o pty_master_open.o

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
