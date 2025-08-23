include ../Makefile.inc

GEN_EXE = pthread_xfr psem_create psem_post psem_wait psem_timedwait posix_sem_test \
        benchmark_posix_sem benchmark_sysv_sem
LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

posix_sem_test : posix_sem_test.c posix_sem.o posix_sem.h
	${CC} ${CFLAGS} -o $@ posix_sem_test.c posix_sem.o ${LDLIBS}

posix_sem.o : posix_sem.c posix_sem.h
	${CC} ${CFLAGS} -c posix_sem.c

benchmark_posix_sem : benchmark_posix_sem.c ${TLPI_LIB}
	${CC} ${CFLAGS} -o $@ benchmark_posix_sem.c ${TLPI_LIB} ${LDLIBS} -lrt

benchmark_sysv_sem : benchmark_sysv_sem.c ${TLPI_LIB}
	${CC} ${CFLAGS} -o $@ benchmark_sysv_sem.c ${TLPI_LIB} ${LDLIBS}
