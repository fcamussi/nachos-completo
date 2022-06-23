#include "synchconsole.h"



/* Constructor */
SynchConsole::SynchConsole(const char* debugName)
{
    name = debugName;
    console = new Console(NULL, NULL, ReadAvail, WriteDone, this);
    readAvail = new Semaphore(debugName, 0);
    writeDone = new Semaphore(debugName, 0);
    lockRead = new Lock(debugName);
    lockWrite = new Lock(debugName);
}


/* Destructor */
SynchConsole::~SynchConsole()
{
    delete console;
    delete readAvail;
    delete writeDone;
    delete lockRead;
    delete lockWrite;
}


/* Lee desde la consola de forma sincronizada, es decir, se permite una sola
   lectura a la vez y espera a que los datos esten listos antes de retornar */
int SynchConsole::Read(char *buffer, int size)
{
    int c;

    lockRead->Acquire();
    for (c = 0; c < size; c++)
    {
	    readAvail->P();	// Espero a que llegue un caracter
	    *buffer = console->GetChar();
        buffer++;
    }
    lockRead->Release();

    return c;
}


/* Escribe en la consola de forma sincronizada, es decir, se permite una sola
   escritura a la vez y espera a que los datos se hayan escrito antes de retornar */
int SynchConsole::Write(char *buffer, int size)
{
    int c;

    lockWrite->Acquire();
    for (c = 0; c < size; c++)
    {
	    console->PutChar(*buffer);
	    writeDone->P(); // Espero que la escritura finalice
        buffer++;
    }
    lockWrite->Release();

    return c;
}


/* Handler que es llamado cuando hay un caracter disponible para ser leido */
static void ReadAvail(void *arg)
{
    SynchConsole *synchConsole = (SynchConsole *)arg;

    synchConsole->readAvail->V();
}


/* Handler que es llamado cuando el caracter fue escrito */
static void WriteDone(void *arg)
{
    SynchConsole *synchConsole = (SynchConsole *)arg;

    synchConsole->writeDone->V();
}


