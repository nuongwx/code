#include "header.h"

void copy()
{
    int fd, fd2, read;

    Write("\ncopy(): enter source file name: ", 33, STDOUT);
    Read(buffer, BUFSIZ, STDIN);

    fd = Open(buffer, 1);
    if (fd == -1)
    {
        Write("\ncopy(): cannot open source file\n", 33, STDOUT);
        return;
    }

    Write("\ncopy(): enter dest file name: ", 31, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    if (Create(buffer))
    {
        Write("\ncopy(): cannot create dest file\n", 33, STDOUT);
        return;
    }
    else if ((fd2 = Open(buffer, 1)) > 0)
    {
        do
        {
            read = Read(buffer, BUFSIZ, fd);
            Write(buffer, read != -2 ? read : Seek(-1, fd) % BUFSIZ, fd2);
        } while (read > 0);
        Close(fd);
        Close(fd2);
        return;
    }
    else
    {
        Write("\ncopy(): cannot open dest file for write\n", 41, STDOUT);
        return;
    }
}

int main()
{
    copy();
}