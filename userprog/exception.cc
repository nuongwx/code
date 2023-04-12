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

#define MAX_STRING_SIZE 1023

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
        // DEBUG('k', "%c", kernelBuf[i]);
        // if (oneChar == 0)
        //     break;
    }
    DEBUG('k', "\nkernelBuf: %s\n", kernelBuf);
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
    } while (i < len); // && oneChar != 0);
    DEBUG('k', "\nSystem2User: %s", buffer);
    return i;
}

void Halt()
{
    DEBUG('z', "\n Shutdown, initiated by user program.");
    DEBUG('z', "\n\n Shutdown, initiated by user program.");
    interrupt->Halt();
}

void PCIncrease()
{
    int pc;
    pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc); // PrevPC <- PC
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc); // PC <- NextPC
    pc += 4;
    machine->WriteRegister(NextPCReg, pc); // NextPC <- NextPC + 4
}

void printMem(char *c)
{
    DEBUG('z', "\n MEM DUMP: ");

    for (int i = 0; i < 50; i++)
        DEBUG('z', "%d ", c[i]);
}

/*
    file n stuff
*/

/**
 * @brief Open a file
 * @param name: file name
 * @param mode: 0 for O_RDONLY, 1 for O_RDWR
 * @return index of file in OpenFileTable, -1 if fail
 */
void Open()
{
    int virtAddr = machine->ReadRegister(4);
    int mode = machine->ReadRegister(5);

    char *filename = User2System(virtAddr, MAX_STRING_SIZE + 1);
    DEBUG('z', "\nDUMP: %s", filename);
    if (filename == NULL)
    {
        DEBUG('z', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    int ret = fileSystem->fOpen(filename, mode);
    DEBUG('z', "\n Open file %s, mode %d, return id %d", filename, mode, ret);
    // if (filename)   // seems like ASSERT() gets called when filename is NULL, thus not needed for now
    machine->WriteRegister(2, ret);
    delete filename;
}

/**
 * @brief Close a file
 * @param id: index of file in OpenFileTable
 * @return 0 if success, -1 if fail
 */
void Close()
{
    int id = machine->ReadRegister(4);
    machine->WriteRegister(2, fileSystem->fClose(id));
}

/**
 * @brief Read fd
 * @param buffer: pointer to buffer
 * @param size: number of bytes to read
 * @param id: index of file in OpenFileTable
 * @return number of bytes read, -1 if fail, -2 if EOF reached
 */
void Read()
{
    int virtAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int id = machine->ReadRegister(6);

    char *buffer = new char[size + 1];
    DEBUG('z', "\n TO READ: %d bytes from file %d\n", size, id);
    int ret = fileSystem->fRead(buffer, size, id);
    if (ret < size)
    {
        ret = -2;
    }
    System2User(virtAddr, size, buffer);
    DEBUG('z', "\n CONTINUE %d", ret);
    printMem(buffer);
    machine->WriteRegister(2, ret);
    delete buffer;
}

/**
 * @brief Write fd
 * @param buffer: pointer to buffer
 * @param size: number of bytes to write
 * @param id: index of file in OpenFileTable
 * @return number of bytes written, -1 if fail
 */
void Write()
{
    int virtAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int id = machine->ReadRegister(6);
    DEBUG('z', "\n Write: %d bytes to file %d\n", size, id);
    char *buffer = User2System(virtAddr, size);
    DEBUG('z', "\nBUFF DUMP: ");
    printMem(buffer);
    if (buffer == NULL)
    {
        DEBUG('z', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete buffer;
        return;
    }
    int ret = fileSystem->fWrite(buffer, size, id);
    if (ret < size)
    {
        ret = -2;
    }
    machine->WriteRegister(2, ret);
    delete buffer;
    // return PCIncrease();
}

/**
 * @brief Seek to position in file
 * @param pos: position to seek to
 * @param id: index of file in OpenFileTable
 * @return 0 if success, -1 if fail
 */
void Seek()
{
    int pos = machine->ReadRegister(4);
    int id = machine->ReadRegister(5);
    machine->WriteRegister(2, fileSystem->fSeek(pos, id));
}

/**
 * @brief Delete a file
 * @param name: file name
 * @return 0 if success, -1 if file's not exist, 1 if UNIX unlink() fails and file is not removed
 */
void Delete()
{
    int virtAddr = machine->ReadRegister(4);
    char *filename = User2System(virtAddr, MAX_STRING_SIZE + 1);
    if (filename == NULL)
    {
        DEBUG('z', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    DEBUG('z', "\n filename: %s", filename);
    if (fileSystem->fDelete(filename))
    {
        DEBUG('z', "\n Error delete file '%s'", filename);
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    machine->WriteRegister(2, 0);
    delete filename;
}

/**
 * @brief Create a file
 * @param name: file name
 * @return 0 if success, -1 if fail
 * @note: file size is 0, overwrite if exist
 */
void Create() // Create(char *name)
{
    int virtAddr = machine->ReadRegister(4);
    char *filename = User2System(virtAddr, MAX_STRING_SIZE + 1);
    DEBUG('z', "\nENTRY: ");
    printMem(filename);

    if (filename == NULL)
    {
        DEBUG('z', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    DEBUG('z', "\n\nCREAT filename: %s\n", filename);
    if (!fileSystem->Create(filename, 0))
    {
        DEBUG('z', "\n Error create file '%s'", filename);
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    machine->WriteRegister(2, 0);
    delete filename;
}

void StartProcess(int virtAddr)
{
    currentThread->space->InitRegisters(); // set the initial register values
    currentThread->space->RestoreState();  // load page table register
    machine->Run();                        // jump to the user progam
    ASSERT(FALSE);                         // machine->Run never returns;
                                           // the address space exits
                                           // by doing the syscall "exit"
}

void Exit()
{
    int exitCode = machine->ReadRegister(4);
    DEBUG('z', "\n Exit code: %d", exitCode);
    if (strcmp(currentThread->getName(), "main") == 0)
    {
        interrupt->Halt();
    }
    else
    {
        currentThread->Finish();
    }
}

/**
 * @brief Execute a program in a new thread
 * @param name: file name
 * @return SpaceId if success, -1 if fail
 * @note: Assume file is executable
 */
void Exec()
{
    int virtAddr = machine->ReadRegister(4);
    char *filename = User2System(virtAddr, MAX_STRING_SIZE + 1);
    if (filename == NULL)
    {
        DEBUG('z', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    DEBUG('r', "\n Exec filename: %s", filename);
    OpenFile *executable = fileSystem->Open(filename);
    if (executable == NULL)
    {
        DEBUG('z', "\n Unable to open file %s", filename);
        machine->WriteRegister(2, -1);
        delete filename;
        return;
    }
    AddrSpace *space = new AddrSpace(executable);
    delete executable; // close file
    Thread *newThread = new Thread(filename);
    newThread->space = space;
    newThread->priority = machine->ReadRegister(5);
    newThread->Fork(StartProcess, virtAddr);
    machine->WriteRegister(2, newThread->space->spaceId);
    DEBUG('r', "\n Exec filename: %s, spaceId: %d", filename, newThread->space->spaceId);
    // delete filename;
}

void PrintString()
{
    int virtAddr = machine->ReadRegister(4);
    char *buffer = User2System(virtAddr, MAX_STRING_SIZE + 1);
    if (buffer == NULL)
    {
        DEBUG('z', "\n Not enough memory in system");
        machine->WriteRegister(2, -1);
        delete buffer;
        return;
    }
    // printf("%s", buffer);
    gSynchConsole->Write(buffer, strlen(buffer) + 1);
    // return PCIncrease();
}

void Sub() // wee example, just for fun
{
    int op1 = machine->ReadRegister(4);
    int op2 = machine->ReadRegister(5);
    int result = op1 - op2;
    machine->WriteRegister(2, result);
}

/* Footnotes:
    0. "em chỉ xem ý tưởng thôi mà™ 😔"

    0.5. in case you haven't noticed, every joke is a sticky note

    1. There's no 2

    3. The system call number is passed in the r2 register
    4. The system call arguments are passed in the r4, r5, r6, r7 registers

    5. The return value of the system call is passed in the r2 register
    6. The return address of the system call is passed in the r4 register


    tao file co nd echo createfile copy
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
            Halt();
            break;
        case SC_Exit:
            Exit();
            break;
        case SC_Exec:
            Exec();
            break;
        case SC_Create:
            Create();
            break;
        case SC_Open:
            Open();
            break;
        case SC_Close:
            Close();
            break;
        case SC_Read:
            Read();
            break;
        case SC_Write:
            Write();
            break;
        case SC_Seek:
            Seek();
            break;
        case SC_Delete:
            Delete();
            break;
        case SC_PrintString:
            PrintString();
            break;
        default:
            DEBUG('z', "\nUnexpected syscall (%d %d)\n", which, type);
            interrupt->Halt();
            break;
        }
        return PCIncrease();

        break;
    default:
        DEBUG('z', "\nUnexpected user mode exception (%d %d)\n", which, type);
        interrupt->Halt();
        break;
    }
    interrupt->Halt();
}