#ifndef TEST_H
#define TEST_H

#include "syscall.h"
#include "copyright.h"

#define BUFSIZ 254 // arbitrary buffer size really

#define STDIN 0
#define STDOUT 1

char buffer[BUFSIZ + 1];

#endif // TEST_H