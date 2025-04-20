#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    int res;

    switch(fork()) {
        case -1:
            errExit("fork");
        case 0: // child
            if (setpgid(0, 0) == -1) // create a new process group - become a process group leader
                errExit("setpgid");
            
            if ((res = setsid()) == -1)
                errExit("setsid");

            printf("setsid called successfully"); // should not happen
            exit(EXIT_SUCCESS);
                
        default: // parent
        
        exit(EXIT_SUCCESS);
    }
}
