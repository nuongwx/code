#include "syscall.h"
#include "copyright.h"

#define STDIN 0
#define STDOUT 1

#define BUFSIZ 254
char buffer[BUFSIZ + 1];

void print(int n)
{
    int i = 0;
    for (i = 0; i < BUFSIZ; i++)
        buffer[i] = ' ';
    i = 5;
    if (n < 0)
    {
        n = -n;
        buffer[0] = '-';
    }
    while (n > 0)
    {
        buffer[--i] = n % 10 + '0';
        n /= 10;
    }
    buffer[5] = '\n';
    buffer[6] = '\0';
    Write(buffer, BUFSIZ, STDOUT);
}

void echo()
{
    while (Read(buffer, BUFSIZ, STDIN) > 0)
    {
        Write(buffer, BUFSIZ, STDOUT);
    }
    Write(buffer, BUFSIZ, STDOUT);
    PrintString("\n");
}

void cat()
{
    int fd, read;

    Write("cat(): enter file name: ", 24, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    fd = Open(buffer, 1);
    print(fd);

    if (fd < 0)
        return;

    do
    {
        read = Read(buffer, BUFSIZ, fd);
        Write(buffer, BUFSIZ, STDOUT);
    } while (read > 0);
    // Write(buffer, BUFSIZ, STDOUT);
    PrintString("\n"); // try to flush buffer

    Close(fd);
}

void copy()
{
    int fd, fd2, read;

    Write("copy: enter file name: ", 24, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    fd = Open(buffer, 1);
    if (fd == -1)
    {
        return;
    }
    Write("\nlt: enter file name: ", 24, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    if (!Create(buffer))
    {
        Write("copy(): cannot create dest file", 31, STDOUT);
        return;
    }
    if ((fd2 = Open(buffer, 1)) > 0)
    {
        do
        {
            read = Read(buffer, BUFSIZ, fd);
            Write(buffer, BUFSIZ, fd2);
        } while (read > 0);
        // Write(buffer, BUFSIZ, fd2);

        Close(fd);
        Close(fd2);
    }
    else
    {
        Write("copy(): cannot open dest file for write\n", 27, STDOUT);
        return;
    }
}

void delete ()
{
    int fd;

    Write("delete(): enter file name: ", 26, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    fd = Open(buffer, 1);
    // print(fd);
    Close(fd);
    print(Delete(buffer));
}

int main()
{
    copy();
    cat();
    delete ();
    echo();
    Halt();
}