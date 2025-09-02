include ../Makefile.inc

GEN_EXE = slow_reader us_abs_xfr_sv us_xfr_sv us_xfr_cl us_abs_xfr_cl \
	us_seqnum_server us_seqnum_client datagram_connect_test

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

us_xfr_sv.o us_xfr_cl.o : us_xfr.h
us_abs_xfr_sv.o us_abs_xfr_cl.o : us_abs_xfr.h
us_seqnum_server.o us_seqnum_client.o : us_seqnum.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
