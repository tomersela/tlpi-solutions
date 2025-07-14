include ../Makefile.inc

GEN_EXE = svshm_xfr_writer svshm_xfr_reader svshm_xfr_writer_mod svshm_xfr_reader_mod \
			svshm_mon svshm_ls

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

CFLAGS += -pthread -Wall -Wextra -g

DIR_SERVICE_DIR := dir_service
DIR_SERVICE_TARGETS := init_dir set get delete

BUF_SIZE ?= 1024            # default if caller doesn't override
CPPFLAGS += -DBUF_SIZE=$(BUF_SIZE)

all : ${EXE}
	${MAKE} -C $(DIR_SERVICE_DIR)

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP}

svshm_xfr_writer_mod: vms_flags.o
svshm_xfr_reader_mod: vms_flags.o

# Helper libraries
vms_flags.o : vms_flags.c

# Link all bandwidth programs with the following libs
${GEN_EXE} : binary_sems.o

clean :
	${RM} ${EXE} *.o
	${MAKE} -C $(DIR_SERVICE_DIR) clean

showall :
	@ echo ${EXE}
	@echo "dir_service: $(DIR_SERVICE_TARGETS)"

${EXE} : ${TLPI_LIB}		# True as a rough approximation
