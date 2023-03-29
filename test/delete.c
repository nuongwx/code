#include "header.h"

void delete ()
{
    Write("\ndelete(): enter file name: ", 27, STDOUT);
    Read(buffer, BUFSIZ, STDIN);
    if (Delete(buffer))
    {
        Write("\ndelete(): remove file failed\n", 29, STDOUT);
    }
    else
    {
        Write("\ndelete(): remove file success\n", 31, STDOUT);
    }
}

int main()
{
    delete ();
}