#include "syscall.h"
#include "copyright.h"

#define STDIN 0
#define STDOUT 1

#define BUFSIZ 254

void print(int n)
{
    char buffer[BUFSIZ + 1];
    int i = 0;
    for (i = 0; i < BUFSIZ; i++)
        buffer[i] = ' ';
    i = 5;
    if (n < 0)
        n = -n;
    while (n > 0)
    {
        buffer[--i] = n % 10 + '0';
        n /= 10;
    }
    buffer[5] = '\0';
    Write(buffer, BUFSIZ, STDOUT);
}

int main()
{
    char buffer[BUFSIZ + 1];
    int fd;
    int fd2;
    int read;
    Write("cat(): enter file name: ", 24, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    fd = Open(buffer, 0);
    Write("cat(): enter file name: ", 24, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    fd2 = Open(buffer, 2);

    while ((read = Read(buffer, BUFSIZ, fd)) > 0)
    {
        Write(buffer, read, fd2);
    }
    Close(fd);
    Close(fd2);

    Halt();
}