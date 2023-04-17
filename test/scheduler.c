#include "header.h"

int main() // rs = 563
{
    int pingPID, pongPID;
    PrintString("Ping-pong test starting...\n\n");
    pingPID = Exec("/home/ngwx/nachos/nachos-3.4/code/test/ping", 4);
    pongPID = Exec("/home/ngwx/nachos/nachos-3.4/code/test/pong", 4);
    Read(buffer, 1, 0);
}
