#include <unistd.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    const char *lines[] = {
        "Once upon a time, in a land of buffers and streams,",
        "there lived a program that wanted to share its dreams.",
        "But every time it spoke through a pipe so narrow,",
        "its words got stuck like a jammed arrow.",
        "Until one day, a pty came to save,",
        "and freed the words to flow like a wave.",
        "Now line by line, the story can be told,",
        "thanks to unbuffer, brave and bold!",
        NULL
    };
    
    int delay_ms = 300;

    for (int i = 0; lines[i] != NULL; i++) {
        printf("%s\n", lines[i]);
        usleep(delay_ms * 1000);
    }

    return 0;
}
