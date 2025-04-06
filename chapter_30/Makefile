include ../Makefile.inc

GEN_EXE = thread_incr thread_incr_mod test_threadsafe_tree

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}

test_threadsafe_tree : test_threadsafe_tree.o threadsafe_tree.o ${TLPI_LIB}
	${CC} ${CFLAGS} test_threadsafe_tree.o threadsafe_tree.o -o test_threadsafe_tree ${LDLIBS}

test_threadsafe_tree.o : test_threadsafe_tree.c threadsafe_tree.h
threadsafe_tree.o : threadsafe_tree.c threadsafe_tree.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
