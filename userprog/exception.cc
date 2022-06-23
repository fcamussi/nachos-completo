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
#include "system.h"
#include "syscall.h"
#include "exception.h"
#include "copy.h"

#ifdef VM
#include "paging.h"
#endif


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

void ExceptionHandler(ExceptionType which)
{
    switch (which)
	{
        case NoException: // Everything ok!
 		break;
        case SyscallException: // A program executed a system call.
            SysCall();
		break;
        case PageFaultException: // No valid translation found
#ifdef VM
            PageFaultHandler();
#else
            ASSERT(false);
#endif
		break;
        case ReadOnlyException: // Write attempted to page marked "read-only"
            printf("Excepcion 'ReadOnlyException' recibida. Finalizando el proceso %s...\n",
                   currentThread->getName());
            currentThread->Finish();
		break;
        case BusErrorException: // Translation resulted in an invalid physical address
            printf("Excepcion 'BusErrorException' recibida. Finalizando el proceso %s...\n",
                   currentThread->getName());
            currentThread->Finish();
		break;
        case AddressErrorException: // Unaligned reference or one that was beyond the end of the address space
            printf("Excepcion 'AddressErrorException' recibida. Finalizando el proceso %s...\n",
                   currentThread->getName());
            currentThread->Finish();
		break;
        case OverflowException: // Integer overflow in add or sub.
            printf("Excepcion 'OverflowException' recibida. Finalizando el proceso %s...\n",
                   currentThread->getName());
            currentThread->Finish();
		break;
        case IllegalInstrException: // Unimplemented or reserved instr.
            printf("Excepcion 'IllegalInstrException' recibida. Finalizando el proceso %s...\n",
                   currentThread->getName());
            currentThread->Finish();
		break;
		default:
		    printf("Unexpected user mode exception %d\n", which);
        	ASSERT(false);
    }

    /* No hay que modificar el punto de retorno ya que tiene que retomar
       en la misma instrucción que genero la excepción */
}
//----------------------------------------------------------------------

static void SysCall()
{
    int type = machine->ReadRegister(2);

    switch (type)
    {
        case SC_Halt:
            SysCallHalt();
        break;
        case SC_Exit:
            SysCallExit();
        break;
        case SC_Exec:
            SysCallExec();
        break;
        case SC_Join:
            SysCallJoin();
        break;
        case SC_Create:
            SysCallCreate();
        break;
        case SC_Open:
            SysCallOpen();
        break;
        case SC_Read:
            SysCallRead();
        break;
        case SC_Write:
            SysCallWrite();
        break;
        case SC_Close:
            SysCallClose();
        break;
        case SC_ListProc:
            SysListProc();
        break;
        default:
            DEBUG('s', "SysCall %d no implementada llamada desde %s: No hacemos nada\n",
                  type, currentThread->getName());
    }

    IncrementPC(); // Modifico el punto de retorno
}


static void SysCallHalt()
{
    DEBUG('s', "SysCall 'Halt' llamada desde %s: ¡Adios mundo cruel!\n",
          currentThread->getName());
    interrupt->Halt();
}


static void SysCallExit()
{
    int arg1 = machine->ReadRegister(4); // status = return value

    currentThread->SetRetValue(arg1);
    DEBUG('s', "SysCall 'Exit' llamada desde %s: Finalizando el hilo con id %d\n",
          currentThread->getName(), currentThread->GetId());
    currentThread->Finish();
}


static void SysCallExec()
{
    int arg1 = machine->ReadRegister(4); // nombre de fichero
    int arg2 = machine->ReadRegister(5); // argv
    int arg3 = machine->ReadRegister(6); // join
    SpaceId id = -1;
    Thread *thread;
    Arguments *arguments = NULL;

    if (processTable->IsFull())
    {
        DEBUG('s', "SysCall 'Exec' llamada desde %s: Error, tabla de procesos llena\n",
              currentThread->getName());
    }
    else
    {
        char *filename = new char[100];
        CopyStringFromUser(arg1, 100, filename);
        thread = new Thread(filename, arg3 ? true: false); // Nuevo hilo
        id = processTable->AddProcess(thread); // obtengo un id
        thread->SetId(id);
        if (arg2 != 0) // si argv != NULL
        {
            arguments = new Arguments;
            arguments->CopyFromUser(arg2); // leemos y almacenamos los argumentos
        }
        DEBUG('s', "SysCall 'Exec' llamada desde %s: Hilo creado para ejecutar %s con id %d\n",
              currentThread->getName(), filename, id);
        thread->Fork(Fork, arguments); // hago el fork
        delete [] filename;
    }

    machine->WriteRegister(2, id);
}


static void SysCallJoin()
{
    int arg1 = machine->ReadRegister(4); // id
    Thread *thread;
    int retValue = -1;

    if (processTable->IsInUse(arg1))
    {
        thread = processTable->GetProcess(arg1);
        if (thread->GetThreadParent() == currentThread)
        {
            DEBUG('s', "SysCall 'Join' llamada desde %s: Esperando hilo con id %d\n",
                  currentThread->getName(), arg1);
            thread->Join();
            DEBUG('s', "SysCall 'Join': Hilo hijo con id %d finalizado, continuamos\n",
                  thread->GetId());
            retValue = thread->GetRetValue();
        }
        else
        {
            DEBUG('s', "SysCall 'Join' llamada desde %s: Error, el hilo con id %d no es un hijo\n",
                  currentThread->getName(), arg1);
        }
    }
    else
    {
        DEBUG('s', "SysCall 'Join' llamada desde %s: Error, no existe hilo ejecutandose con id %d\n",
              currentThread->getName(), arg1);
    }

    machine->WriteRegister(2, retValue); // retornamos el status del hijo (o -1 si hubo un error)
}


static void SysCallCreate()
{
    int arg1 = machine->ReadRegister(4); // nombre de fichero
    char *filename = new char[100];

    CopyStringFromUser(arg1, 100, filename);
    if (! fileSystem->Create(filename, 0)) // longitud 0
    {
        DEBUG('s', "SysCall 'Create' llamada desde %s: Error al crear el fichero %s\n",
              currentThread->getName(), filename);
    }
    else
    {
        DEBUG('s', "SysCall 'Create' llamada desde %s: El fichero '%s' fue creado\n",
              currentThread->getName(), filename);
    }
    delete [] filename;
}


static void SysCallOpen()
{
    int arg1 = machine->ReadRegister(4); // nombre de fichero
    char *filename = new char[100];
    OpenFile *of;
    OpenFileId id = -1;
    OpenFileTable *openFileTable = currentThread->GetOpenFileTable();

    if (openFileTable->IsFull())
    {
        DEBUG('s', "SysCall 'Open' llamada desde %s: Error, tabla de ficheros abiertos llena\n",
              currentThread->getName());
    }
    else
    {
        CopyStringFromUser(arg1, 100, filename);
        of = fileSystem->Open(filename);
        if (! of)
        {
            DEBUG('s', "SysCall 'Open' llamada desde %s: Error al abrir el fichero %s\n",
                  currentThread->getName(), filename);
        }
        else
        {
            id = openFileTable->AddOpenFile(of);
            DEBUG('s', "SysCall 'Open' llamada desde %s: El fichero '%s' fue abierto con id %d\n",
                  currentThread->getName(), filename, id);
        }
    }
    delete [] filename;

    machine->WriteRegister(2, id); // retorno el id del fichero
}


static void SysCallRead()
{
    int arg1 = machine->ReadRegister(4); // buffer del usuario
    int arg2 = machine->ReadRegister(5); // cantidad a leer
    int arg3 = machine->ReadRegister(6); // id del fichero
    OpenFile *of;
    char *buffer = new char[arg2];
    int readed = 0;
    OpenFileTable *openFileTable = currentThread->GetOpenFileTable();

    if (arg3 == ConsoleInput)
    {
        readed = synchConsole->Read(buffer, arg2);
        CopyToUser(arg1, readed, buffer);
        DEBUG('s', "SysCall 'Read' llamada desde %s: Se leyeron %d caracteres desde Console Input\n",
              currentThread->getName(), readed);
    }
    else
    {
        if(arg3 < 2 || ! openFileTable->IsInUse(arg3))
        {
            DEBUG('s', "SysCall 'Read' llamada desde %s: Error, imposible leer desde el fichero con id %d\n",
                  currentThread->getName(), arg3);
        }
        else
        {
            of = openFileTable->GetOpenFile(arg3);
            readed = of->Read(buffer, arg2);
            CopyToUser(arg1, readed, buffer);
            DEBUG('s', "SysCall 'Read' llamada desde %s: Se leyeron %d bytes desde el fichero con id %d\n",
                  currentThread->getName(), readed, arg3);
        }
    }
    delete [] buffer;

    machine->WriteRegister(2, readed); // retorno la cantidad de bytes leidos
}


static void SysCallWrite()
{
    int arg1 = machine->ReadRegister(4); // buffer del usuario
    int arg2 = machine->ReadRegister(5); // cantidad a escribir
    int arg3 = machine->ReadRegister(6); // id del fichero
    char *buffer = new char[arg2];
    OpenFile *of;
    int written;
    OpenFileTable *openFileTable = currentThread->GetOpenFileTable();

    if (arg3 == ConsoleOutput)
    {
        CopyFromUser(arg1, arg2, buffer);
        written = synchConsole->Write(buffer, arg2);
        DEBUG('s', "SysCall 'Write' llamada desde %s: Se escribieron %d caracteres en Console Output\n",
              currentThread->getName(), written);
    }
    else
    {
        if (arg3 < 2 || ! openFileTable->IsInUse(arg3))
        {
            DEBUG('s', "SysCall 'Write' llamada desde %s: Error, imposible escribir en el fichero con id %d\n",
                  currentThread->getName(), arg3);
        }
        else
        {
            CopyFromUser(arg1, arg2, buffer);
            of = openFileTable->GetOpenFile(arg3);
            written = of->Write(buffer, arg2);
            DEBUG('s', "SysCall 'Write' llamada desde %s: Se escribieron %d bytes en el fichero con id %d\n",
                  currentThread->getName(), written, arg3);
        }
    }
    delete [] buffer;
}


static void SysCallClose()
{
    int arg1 = machine->ReadRegister(4); // id del fichero
    OpenFile *of;
    OpenFileTable *openFileTable = currentThread->GetOpenFileTable();

    if (arg1 < 2 || ! openFileTable->IsInUse(arg1))
    {
        DEBUG('s', "SysCall 'Close' llamada desde %s: Error, imposible cerrar el fichero con id %d\n",
              currentThread->getName(), arg1);
    }
    else
    {
        of = openFileTable->GetOpenFile(arg1);
        openFileTable->RemoveOpenFile(arg1);
        delete of;
        DEBUG('s', "SysCall 'Close' llamada desde %s: El fichero con id %d fue cerrado\n",
              currentThread->getName(), arg1);
    }
}


// Lista los procesos que estan cargados actualmente
static void SysListProc()
{
    int arg1 = machine->ReadRegister(4); // lpValue
    int arg2 = machine->ReadRegister(5); // puntero a estructura LP
    // La primera vez que se llama a ListProc lpValue tiene que ser 0
    // En las siguientes llamadas tiene que ser el valor que retorna
    // la misma función en la llamada anterior
    // En cada llamada se copia en espacio de usuario la estructura
    // LP con información sobre un proceso
    // Como la llamada puede ser hecha de forma simultanea por otro
    // proceso el usuario es quien guarda la posición del recorrido
    // en la tabla de procesos.
    // Cuando no hay más procesos se retorna -1 y no se escribe
    // nada en el espacio de usuario.
    struct LP lp;

    if (arg1 < 0) arg1 = 0;

    while (arg1 < MAX_PROCESSES && ! processTable->IsInUse(arg1))
    {
        arg1++;
    }
    if (arg1 < MAX_PROCESSES)
    {
        Thread *thread = processTable->GetProcess(arg1);

        lp.id = arg1;
        strncpy(lp.name, thread->getName(), sizeof(lp.name));
        lp.status = thread->getStatus();
        DEBUG('s', "SysCall 'ListProc' llamada desde %s: Se copia LP para el proceso id %d\n",
              currentThread->getName(), arg1);
        CopyToUser(arg2, sizeof(struct LP), (char *)&lp);
        machine->WriteRegister(2, arg1 + 1); // posición actual del recorrido
    }
    else
    {
        DEBUG('s', "SysCall 'ListProc' llamada desde %s: No hay mas procesos\n",
              currentThread->getName(), arg1);
        machine->WriteRegister(2, -1); // -1 indica que no hay más procesos
    }
}


/* Incrementa el Program Counter */
static void IncrementPC()
{
    int pc;

    pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc); // PC anterior
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc); // PC actual
    pc += 4; // Todas las instrucciones son palabras de 4 bytes
    machine->WriteRegister(NextPCReg, pc); // PC siguiente (para Delayed Branch)
}


/* Función con la cual se hace el fork */
static void Fork(void *arg)
{
    Arguments *arguments = (Arguments *)arg;

    StartProcess(currentThread->getName(), arguments); // Si algo falla, StartProcess retorna y el proceso finaliza
}


/* Hace las comprobaciones necesarias y se carga el ejecutable en el
   espacio de direcciones (si no se usa VM) junto a los argurmentos si
   es que los hay. Luego se inicializan los registros y se pone en marcha */
void StartProcess(const char *filename, Arguments *arguments)
{
    OpenFile *executable;
    NoffHeader noffH;
    int size;
    int numPages;
    AddrSpace *space;

    executable = fileSystem->Open(filename);
    if (executable == NULL)
    {
        printf("Error al abrir el fichero %s\n", filename);
    }
    else
    {
        executable->ReadAt((char *)&noffH, sizeof(noffH), 0); // leo la cabecera

        // Hago swap de la cabecera si es necesario (endianness)
        if ((noffH.noffMagic != NOFFMAGIC) && 
            (WordToHost(noffH.noffMagic) == NOFFMAGIC))
            SwapHeader(&noffH);

        if (noffH.noffMagic != NOFFMAGIC) // verifico el número mágico
        {
            printf("El fichero %s no es un ejecutable de NachOS™\n", filename);
            delete executable;
        }
        else
        {
            // how big is address space?
            size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
                   + UserStackSize;	// we need to increase the size to leave room for the stack

            numPages = divRoundUp(size, PageSize);
            DEBUG('s', "StartProcess: Cantidad de páginas del programa %s: %d\n", filename, numPages);

            // check we're not trying to run anything too big
            // at least until we have virtual memory
#ifndef VM
            if (numPages > bitMap->NumClear())
            {
                printf("No hay suficientes páginas físicas para cargar %s\n", filename);
                delete executable;
                return;
            }
#endif
            // Se crea un nuevo espacio y se lo inicializa
            space = new AddrSpace(numPages, executable, noffH);
            currentThread->space = space;
            space->InitRegisters(); // set the initial register values
            space->RestoreState(); // load page table register
#ifndef VM
            space->LoadProgram();
#endif
            if (arguments != NULL) // si había argumentos
            {
                space->LoadArguments(arguments); // copiamos los argumentos al nuevo espacio de direcciones
                delete arguments;
            }

            DEBUG('s', "StartProcess: Se procede a ejecutar el programa %s.\n", filename);

            machine->Run();	// jump to the user progam

            // machine->Run never returns, the address space exits by doing the syscall "exit"
            ASSERT(false);
        }
    }
}


