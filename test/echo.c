#include "header.h"

void echo()
{
    while (Read(buffer, BUFSIZ, STDIN) > 0)
    {
        Write(buffer, BUFSIZ, STDOUT);
    }
    Write(buffer, BUFSIZ, STDOUT);
    PrintString("\n");
}

int main()
{
    echo();
}