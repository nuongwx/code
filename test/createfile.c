#include "syscall.h"
#include "copyright.h"
int main()
{
    if (Create("test.txt"))
        PrintString("Create file successfully\n");
    else
        PrintString("Create file failed\n");

    Halt();
}