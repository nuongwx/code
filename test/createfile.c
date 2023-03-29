#include "header.h"

void create()
{
    Write("create(): enter file name: ", 27, STDOUT);
    Read(buffer, BUFSIZ, STDIN);

    if (Create(buffer))
    {
        Write("create(): create file failed\n", 29, STDOUT);
    }
    else
    {
        Write("create(): create file success\n", 30, STDOUT);
    }
}

int main()
{
    create();
}