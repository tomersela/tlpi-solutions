include ../Makefile.inc

GEN_EXE =

LINUX_EXE = dump_utmpx view_lastlog getlogin utmpx_login utmpx_login_mod login_functions who

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
