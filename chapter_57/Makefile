include ../Makefile.inc

GEN_EXE = slow_reader us_abs_xfr_sv us_xfr_sv us_xfr_cl us_abs_xfr_cl

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

us_xfr_sv.o us_xfr_cl.o : us_xfr.h
us_abs_xfr_sv.o us_abs_xfr_cl.o : us_abs_xfr.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
