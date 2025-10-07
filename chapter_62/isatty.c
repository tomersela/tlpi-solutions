#include <termios.h>

#include "isatty.h"


int
isatty(int fd)
{
    struct termios tios;
    return tcgetattr(fd, &tios) != -1;
}
