include ../Makefile.inc

GEN_EXE = longrunner chmod_failure myrealpath list_files list_files_mod getcwd nftw_dir_tree file_stats myntfw chdir_perf

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}

clean :
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
