#include "assert.h"
#include "stdio.h"

unsigned int div(unsigned int x, unsigned int y);
unsigned int modulo(unsigned int x, unsigned int y);

int main() {
    div(64, 3);
    assert(div(64, 3) == 21);
    assert(modulo(64, 3) == 1);
    printf("tests done \n");
}