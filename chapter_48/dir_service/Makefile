GEN_EXE = init_dir get set delete rm_dir
LINUX_EXE =
EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -Wall -Wextra -g
CPPFLAGS += -I../../lib    # Add this if tlpi_hdr.h is under lib/

LDLIBS += ${IMPL_LDLIBS} ${LINUX_LIBCAP}

all: ${EXE}

allgen: ${GEN_EXE}

${GEN_EXE}: dir_service.o

dir_service.o: dir_service.c dir_service.h
	${CC} ${CFLAGS} ${CPPFLAGS} -c dir_service.c -o dir_service.o

init_dir: init_dir.o dir_service.o
rm_dir: rm_dir.o dir_service.o
get: get.o dir_service.o
set: set.o dir_service.o
delete: delete.o dir_service.o

%.o: %.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c $< -o $@

clean:
	${RM} ${EXE} *.o

showall:
	@echo ${EXE}
