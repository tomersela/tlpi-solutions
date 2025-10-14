/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2025.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 63-1 */

/* select_pipes.c

   Example of the use of select() to monitor multiple file descriptors.

   Usage: select_pipes num-pipes [num-writes]
                                  def = 1

   Create 'num-pipes' pipes, and perform 'num-writes' writes to
   randomly selected pipes. Then use select() to inspect the read ends
   of the pipes to see which pipes are readable.
*/
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    fd_set readfds;
    int numPipes, nfds, ready, randPipe, numWrites, j;
    int (*pfds)[2];                     /* File descriptors for all pipes */


    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-pipes [num-writes]\n", argv[0]);

    /* Allocate the arrays that we use. The arrays are sized according
       to the number of pipes specified on command line */

    numPipes = getInt(argv[1], GN_GT_0, "num-pipes");
    numWrites = (argc > 2) ? getInt(argv[2], GN_GT_0, "num-writes") : 1;

    pfds = calloc(numPipes, sizeof(int [2]));
    if (pfds == NULL)
        errExit("calloc");

    /* Create the number of pipes specified on command line */

    for (j = 0; j < numPipes; j++) {
        if (pipe(pfds[j]) == -1) 
            errExit("pipe %d", j);
    }

    /* Perform specified number of writes to random pipes */

    srandom((int) time(NULL));
    for (j = 0; j < numWrites; j++) {
        randPipe = random() % numPipes;
        printf("Writing to fd: %3d (read fd: %3d)\n",
                pfds[randPipe][1], pfds[randPipe][0]);
        if (write(pfds[randPipe][1], "a", 1) == -1)
            errExit("write %d", pfds[randPipe][1]);
    }

    // /* Build the file descriptor list to be supplied to select(). This list
    //    is set to contain the file descriptors for the read ends of all of
    //    the pipes. */

    nfds = 0;
    FD_ZERO(&readfds);
    for (j = 0; j < numPipes; j++) {
        FD_SET(pfds[j][0], &readfds);
        if (pfds[j][0] >= nfds)
            nfds = pfds[j][0] + 1;
    }

    ready = select(nfds, &readfds, NULL, NULL, NULL);
    
    if (ready == -1)
        errExit("select");

    printf("select() returned: %d\n", ready);

    /* Check which pipes have data available for reading */

    for (j = 0; j < numPipes; j++)
        if (FD_ISSET(pfds[j][0], &readfds))
            printf("Readable: %3d\n", pfds[j][0]);

    exit(EXIT_SUCCESS);
}
