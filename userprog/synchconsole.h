#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "synch.h"



static void ReadAvail(void *arg);
static void WriteDone(void *arg);


class SynchConsole
{
    friend void ReadAvail(void *arg); // estas funciones son declaras amigas para poder
    friend void WriteDone(void *arg); // acceder a los miembros privados de la clase
    public:
        SynchConsole(const char* debugName);
        ~SynchConsole();
        const char* getName() { return name; }
        int Read(char *buffer, int size);
        int Write(char *buffer, int size);
    private:
        const char* name;
        Console *console;
        Semaphore *readAvail;
        Semaphore *writeDone;
        Lock *lockRead;
        Lock *lockWrite;
};



#endif // SYNCHCONSOLE_H
