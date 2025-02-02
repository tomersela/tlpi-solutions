include ../Makefile.inc

GEN_EXE = new_parent ancestry print_wait_status.o child_status child_status_mod make_zombie make_zombie_mod

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}


all : ${EXE}

child_status_mod: print_wait_status.o
child_status: print_wait_status.o

print_wait_status.o: print_wait_status.c print_wait_status.h
	${CC} ${CFLAGS} -c print_wait_status.c -o print_wait_status.o

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
