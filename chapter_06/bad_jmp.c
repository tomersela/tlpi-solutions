#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

static jmp_buf jbuf;

static void f1(void) {
    printf("Calling setjmp\n");
    setjmp(jbuf);
}

int main(int argc, char* argv[]) {
    printf("In main\n");
    printf("Calling f1\n");
    f1();
    printf("Returning from f1\n");
    longjmp(jbuf, 123);
    printf("Back to main\n");
}
