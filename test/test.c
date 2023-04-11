#include "header.h"

int main() // rs = 563
{
    Exec("/home/ngwx/nachos/nachos-3.4/code/test/ping", 1);
    Exec("/home/ngwx/nachos/nachos-3.4/code/test/pong", 0);
    Read(buffer, 10, 0);
}
