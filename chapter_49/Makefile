include ../Makefile.inc

GEN_EXE = cp svshm_xfr_reader svshm_xfr_writer mmap_xfr_reader mmap_xfr_writer \
			segv_test sigbus_test nonlinear

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean :
	${RM} ${EXE} *.o

svshm_xfr_reader.o svshm_xfr_writer.o: svshm_xfr.h

# Link all bandwidth programs with the following libs
${GEN_EXE} : binary_sems.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
