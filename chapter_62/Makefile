include ../Makefile.inc

GEN_EXE = test_tty_functions isatty_test ttyname_test getpass_test canon

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

test_tty_functions: tty_functions.o

tty_functions.o: tty_functions.h

isatty_test: isatty.o

isatty.o: isatty.h

ttyname_test: ttyname.o

ttyname.o: ttyname.h

getpass.o: getpass.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation