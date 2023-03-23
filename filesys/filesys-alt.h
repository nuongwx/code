#ifndef FS_ALT_H
#define FS_ALT_H

#include "openfile.h"
#include "utility.h"

#define GLOBAL_FILE_TABLE_SIZE 10

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

#define RO 0
#define RW 1

/*
    sysdep.h : OpenForRead()
    openfile.something : Seek()
*/

typedef int OpenFileId;

class Node
{
public:
    OpenFile *file;
    int mode;

    Node() : file(NULL), mode(0) {}
    Node(OpenFile *f, int m) : file(f), mode(m) {}
};

// additional features for FileSystem, powered by [REDACTED]
class FileSystemAlt
{
private:
    Node **globalFileTable;
    int count;

public:
    FileSystemAlt() : globalFileTable(new Node *[GLOBAL_FILE_TABLE_SIZE]), count(0)
    {
        for (int i = 0; i < GLOBAL_FILE_TABLE_SIZE; i++)
        {
            globalFileTable[i] = NULL;
        }

        // preserves something something
        globalFileTable[CONSOLE_INPUT] = new Node();
        globalFileTable[CONSOLE_OUTPUT] = new Node();
    }
    ~FileSystemAlt()
    {
        for (int i = 0; i < GLOBAL_FILE_TABLE_SIZE; i++)
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
        default:
            break;
        }
        if (fileDescriptor != -1)
        {
            OpenFile *file = new OpenFile(fileDescriptor);
            Node *node = new Node(file, mode);
            for (int i = 0; i < GLOBAL_FILE_TABLE_SIZE; i++)
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
        if (id < 2 || id >= GLOBAL_FILE_TABLE_SIZE || globalFileTable[id] == NULL)
        {
            return -1;
        }
        // readsize > size eq EOF reached as per OpenFile::Read()
        return (globalFileTable[id]->file->Read(buffer, size) == size) ? size : -2;
    }

    int Write(char *buffer, int size, OpenFileId id)
    {
        if (id < 2 ||
            id >= GLOBAL_FILE_TABLE_SIZE ||
            globalFileTable[id] == NULL ||
            globalFileTable[id]->mode == RO)
        {
            return -1;
        }
        return globalFileTable[id]->file->Write(buffer, size); // checks probably not needed
    }

    int Seek(int pos, OpenFileId id)
    {
        if (id < 2 ||
            id >= GLOBAL_FILE_TABLE_SIZE ||
            globalFileTable[id] == NULL ||
            pos < -1 ||
            pos > globalFileTable[id]->file->Length())
        {
            return -1;
        }
        return (pos == -1) ? globalFileTable[id]->file->Seek(globalFileTable[id]->file->Length()) : globalFileTable[id]->file->Seek(pos);
    }
};

#endif // FS_ALT_H