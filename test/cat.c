#include "header.h"

void cat()
{
    int fd, read;

    Write("\ncat(): enter file name: ", 25, STDOUT);
    Read(buffer, BUFSIZ, STDIN);

    fd = Open(buffer, 1);
    if (fd < 0)
    {
        Write("\ncat(): open file failed\n", 25, STDOUT);
        return;
    }

    do
    {
        read = Read(buffer, BUFSIZ, fd);
        Write(buffer, BUFSIZ, STDOUT); // write to console, not file
    } while (read > 0);

    PrintString("\n");
    Close(fd);
}

int main()
{
    cat();
}