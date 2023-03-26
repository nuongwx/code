#ifndef FS_ALT_H
#define FS_ALT_H

#include "openfile.h"
#include "utility.h"
#include "synchcons.h"

extern SynchConsole *gSynchConsole;

#define GLOBAL_FILE_TABLE_SIZE 10

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

#define RO 0
#define RW 1
#define WO 2 // Create() fix

/*
    sysdep.h : OpenForRead()
*/

typedef int OpenFileId;

class Node
{
public:
    OpenFile *file;
    int mode;
    char *name;

    Node() : file(NULL), mode(0) {}
    Node(OpenFile *f, const char *c, int m) : file(f), mode(m)
    {
        name = new char[strlen(c) + 1];
        strcpy(name, c);
    }
    ~Node()
    {
        delete[] name;
    }
};

// additional features for FileSystem, powered by [REDACTED]
class FileSystemAlt
{
private:
    Node **globalFileTable;
    int count;

public:
    void printMem(char *c)
    {
        DEBUG('z', "\n MEM DUMP: ");

        for (int i = 0; i < 50; i++)
            DEBUG('z', "%d ", c[i]);
    }
    FileSystemAlt() : globalFileTable(new Node *[GLOBAL_FILE_TABLE_SIZE]), count(2)
    {
        for (int i = 2; i < GLOBAL_FILE_TABLE_SIZE; i++)
        {
            globalFileTable[i] = NULL;
        }

        // preserves something something
        globalFileTable[CONSOLE_INPUT] = new Node(0, "stdin", RO);
        globalFileTable[CONSOLE_OUTPUT] = new Node(0, "stdout", WO);
    }
    ~FileSystemAlt()
    {
        for (int i = 2; i < GLOBAL_FILE_TABLE_SIZE; i++)
        {
            if (globalFileTable[i] != NULL)
            {
                delete globalFileTable[i];
            }
        }
        delete[] globalFileTable;
    }

    // TODO: implement syscalls
    int Open(char *name, int mode)
    {
        int fileDescriptor = -1;
        switch (mode)
        {
        case RO:
            fileDescriptor = OpenForRead(name, FALSE);
            break;
        case RW:
            fileDescriptor = OpenForReadWrite(name, FALSE);
            break;
        case WO:
            fileDescriptor = OpenForWrite(name);
            break;
        default:
            break;
        }
        if (fileDescriptor != -1)
        {
            OpenFile *file = new OpenFile(fileDescriptor);
            Node *node = new Node(file, name, mode);
            for (int i = 2; i < GLOBAL_FILE_TABLE_SIZE; i++)
            {
                if (globalFileTable[i] == NULL)
                {
                    globalFileTable[i] = node;
                    count++;
                    return i;
                }
            }
            delete node;
            delete file;
        }
        return -1;
    }

    int Close(OpenFileId id)
    {
        if (id < 2 || id >= GLOBAL_FILE_TABLE_SIZE || globalFileTable[id] == NULL)
        {
            return -1;
        }
        delete globalFileTable[id];
        globalFileTable[id] = NULL;
        count--;
        return 0;
    }

    int Read(char *buffer, int size, OpenFileId id)
    {
        if (id < 0 || id >= GLOBAL_FILE_TABLE_SIZE || globalFileTable[id] == NULL || id == CONSOLE_OUTPUT)
        {
            return -1;
        }
        memset(buffer, 0, size);
        printMem(buffer);
        int read = 0;
        DEBUG('z', "\n\nREAD BEGIN: %s", buffer);

        if (id == CONSOLE_INPUT)
        {
            read = gSynchConsole->Read(buffer, size);
        }
        else
        {
            read = globalFileTable[id]->file->Read(buffer, size);
        }
        buffer[read] = '\0';
        DEBUG('z', "\nDUMP READ: ");
        printMem(buffer);
        // readsize > size eq EOF reached as per OpenFile::Read()
        return (read == size) ? size : -2;
    }

    int Write(char *buffer, int size, OpenFileId id)
    {
        if (id < 0 ||
            id >= GLOBAL_FILE_TABLE_SIZE ||
            globalFileTable[id] == NULL ||
            globalFileTable[id]->mode == RO)
        {
            return -1;
        }
        DEBUG('z', "\n\nWRITE BEGIN: %s", buffer);
        // size = strnlen(buffer, size);
        int read = 0;
        if (id == CONSOLE_OUTPUT)
        {
            read = gSynchConsole->Write(buffer, size);
        }
        else
        {
            read = globalFileTable[id]->file->Write(buffer, size);
        } // checks probably not needed
        DEBUG('z', "\nDUMP WRITE: ");
        printMem(buffer);
        return read;
    }

    int Seek(int pos, OpenFileId id)
    {
        if (id < 2 || // error if fd is console
            id >= GLOBAL_FILE_TABLE_SIZE ||
            globalFileTable[id] == NULL ||
            pos < -1 ||
            pos > globalFileTable[id]->file->Length())
        {
            return -1;
        }
        if (pos == -1)
        {
            pos = globalFileTable[id]->file->Length();
        }
        globalFileTable[id]->file->Seek(pos);
        return Tell(id) == pos ? pos : -1;
    }

    int Find(char *name)
    {
        printf("Find: %s", name);
        for (int i = 2; i < GLOBAL_FILE_TABLE_SIZE; i++)
        {
            if (globalFileTable[i] != NULL && strcmp(globalFileTable[i]->name, name) == 0)
            {
                return i;
            }
        }
        return -1; // not open
    }

    int Delete(char *name)
    {
        return Find(name) == -1 ? Unlink(name) : -1;
    }
};

#endif // FS_ALT_H