// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "syscall.h"
#include "system.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

#define MAX_FILENAME_LENGTH 32
#define MAX_STRING_SIZE 32

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char *User2System(int virtAddr, int limit)
{
    int i; // index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; // need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    // printf("\n Filename u2s:");
    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        // printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

void Halt()
{
    DEBUG('a', "\n Shutdown, initiated by user program.");
    printf("\n\n Shutdown, initiated by user program.");
    interrupt->Halt();
}

void PCIncrease() // why 4? because the instruction is 4 bytes
{
    int pc;
    pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc); // PrevPC <- PC
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc); // PC <- NextPC
    pc += 4;
    machine->WriteRegister(NextPCReg, pc); // NextPC <- NextPC + 4
}

/*
    file n stuff
*/

void Open()
{
}

//    problem: overwriting existing file, by Create()... somehow
void Create() // Create(char *name)
{
    int virtAddr = machine->ReadRegister(4);
    char *filename = User2System(virtAddr, MAX_FILENAME_LENGTH + 1);
    if (filename == NULL)
    {
        printf("\n Not enough memory in system");
        DEBUG('a', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    DEBUG('a', "\n Finish reading filename.");
    if (!fileSystem->Create(filename, 0))
    {
        printf("\n Error create file '%s'", filename);
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    machine->WriteRegister(2, 0);
    delete filename;
    return PCIncrease();
}

void PrintString()
{
    int virtAddr = machine->ReadRegister(4);
    char *buffer = User2System(virtAddr, MAX_STRING_SIZE + 1);
    if (buffer == NULL)
    {
        printf("\n Not enough memory in system");
        DEBUG('a', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete buffer;
        return;
    }
    // printf("%s", buffer);
    gSynchConsole->Write(buffer, strlen(buffer) + 1);
    return PCIncrease();
}

void Sub() // wee example, just for fun
{
    int op1 = machine->ReadRegister(4);
    int op2 = machine->ReadRegister(5);
    int result = op1 - op2;
    machine->WriteRegister(2, result);
    return PCIncrease();
}

/* Footnotes:
    0. "em chỉ xem ý tưởng thôi mà™ 😔"

    1. PCIncrease() must be called after each system call
    2. return PCIncrease() just works.

    3. The system call number is passed in the r2 register
    4. The system call arguments are passed in the r4, r5, r6, r7 registers

    5. The return value of the system call is passed in the r2 register
    6. The return address of the system call is passed in the r4 register
WIP
*/

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    switch (which)
    {
    case NoException:
        return;
    case SyscallException:
        switch (type)
        {
        case SC_Halt:
            return Halt();
        case SC_Create:
            return Create();
        case SC_PrintString:
            return PrintString();
        default:
            printf("\nUnexpected syscall (%d %d)\n", which, type);
            interrupt->Halt();
            break;
        }
        break;
    default:
        printf("\nUnexpected user mode exception (%d %d)\n", which, type);
        interrupt->Halt();
        break;
    }
    interrupt->Halt();
}