include ../Makefile.inc

GEN_EXE = is_seqnum_sv is_seqnum_cl is_seqnum_sv_mod is_seqnum_cl_mod \
	is_seqnum_sv_mod2 is_seqnum_cl_mod2 us_xfr_sv us_xfr_cl \
	us_xfr_sv_mod us_xfr_cl_mod

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

is_seqnum_sv is_seqnum_cl: read_line.o
is_seqnum_sv_mod is_seqnum_cl_mod: read_line_mod.o
is_seqnum_sv_mod2 is_seqnum_cl_mod2: read_line_mod.o inet_sockets.o

is_seqnum_sv.o is_seqnum_cl.o : is_seqnum.h
is_seqnum_sv_mod.o is_seqnum_cl_mod.o : is_seqnum_mod.h
is_seqnum_sv_mod2.o is_seqnum_cl_mod2.o : is_seqnum_mod2.h

read_line.o : read_line.h
read_line_mod.o : read_line_mod.h
read_line_mod2.o : read_line_mod2.h

read_line_mod2.o: inet_sockets.h

inet_sockets.o: inet_sockets.h


us_xfr_sv.o us_xfr_cl.o : us_xfr.h
us_xfr_sv_mod us_xfr_cl_mod: unix_sockets.o

us_xfr_sv_mod.o us_xfr_cl_mod.o : us_xfr.h

unix_sockets.o: unix_sockets.h

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
