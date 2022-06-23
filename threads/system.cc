// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
//#include "preemptive.h"

const int TimeSlice = 500; // cantidad de ticks antes de hacer un cambio de contexto

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;			// the thread we are running now
Thread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *timer;				// the hardware timer device,
					// for invoking context switches
					
// 2007, Jose Miguel Santos Espino
//PreemptiveScheduler* preemptiveScheduler = NULL;
//const long long DEFAULT_TIME_SLICE = 50000;

#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers
SynchConsole *synchConsole;
ProcessTable *processTable;

#ifdef VM
CoreMap *coreMap;
Swap *swap;
#ifdef REPL_AGING 
ReplAging *replAging;
#elif REPL_OPTIMAL
ReplOptimal *replOptimal;
#endif
#else
BitMap *bitMap;
#endif

Timer *preemptionTimer;
#endif

#ifdef NETWORK
PostOffice *postOffice;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();




//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer deviSIGTRAP example -perlce.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each timeALRM there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(void* dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

//----------------------------------------------------------------------
// Preemption
// Llamada cada vez que se incrementa totalTicks
//----------------------------------------------------------------------
static void Preemption(void *dummy)
{
    static int ticks;

    if (ticks > TimeSlice) // si se supera el limite de ticks
    {
        if (interrupt->getStatus() != IdleMode) // si hay algo ejecutandose
        {
            if (scheduler->ReadyNumber() > 0)
            {
                DEBUG('x', "** Cambio de contexto ** (cantidad de procesos ready en el scheduler: %d)\n",
                      scheduler->ReadyNumber());
    	        interrupt->YieldOnReturn(); // se marca para hacer el cambio de contexto al retornar
            }
        ticks = 0; // reseteamos nuestro contador
        }
    }
    ticks++; // incrementamos nuestro contador
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in ALRMorder to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    const char* debugArgs = "";
    bool randomYield = false;
    

// 2007, Jose Miguel Santos Espino
//    bool preemptiveScheduling = false;
//    long long timeSlice;
    
#ifdef USER_PROGRAM
    bool debugUserProg = false;	// single step user program
#endif
#ifdef VM
#ifdef REPL_OPTIMAL
    char *pageRefs;
#endif
#endif
#ifdef FILESYS_NEEDED
    bool format = false;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = true;
	    argCount = 2;
	}
	// 2007, Jose Miguel Santos Espino
/*	else if (!strcmp(*argv, "-p")) {
	    preemptiveScheduling = true;
	    if (argc == 1) {
	        timeSlice = DEFAULT_TIME_SLICE;
	    } else {
	        timeSlice = atoi(*(argv+1));
	        argCount = 2;
	    }
	}*/
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = true;
#endif
#ifdef VM
#ifdef REPL_OPTIMAL
    if (!strcmp(*argv, "-a"))
    {
        ASSERT(argc > 1);
        pageRefs = *(argv + 1);
        argCount = 2;
    }
#endif
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = true;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    argCount = 2;
	}
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);

    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main"); // no joineable con prioridad 0
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
    // Jose Miguel Santos Espino, 2007
/*    if ( preemptiveScheduling ) {
        preemptiveScheduler = new PreemptiveScheduler();
        preemptiveScheduler->SetUp(timeSlice);
    }*/

    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
    synchConsole = new SynchConsole("CONSOLE"); // consola sincronizada
    processTable = new ProcessTable(MAX_PROCESSES); // tabla de procesos creados
    currentThread->SetId(processTable->AddProcess(currentThread)); // proceso main obtiene id 0

#ifdef VM
    coreMap = new CoreMap(NumPhysPages); // coremap para administrar las páginas físicas
    swap = new Swap(MAX_PROCESSES); // espacio de intercambio
#ifdef REPL_AGING 
    replAging = new ReplAging(NumPhysPages); // algoritmo de reemplazo conocido como envejecimiento
#elif REPL_OPTIMAL
    replOptimal = new ReplOptimal(pageRefs); // algoritmo de reemplazo óptimo
#endif
#else
    bitMap = new BitMap(NumPhysPages); // bitmap para administrar las páginas físicas
#endif

    preemptionTimer = new Timer(Preemption, NULL, false); // Timer para time-slicing
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{

    printf("\nCleaning up...\n");

// 2007, Jose Miguel Santos Espino
//    delete preemptiveScheduler;

#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
    delete synchConsole;
    delete processTable;

#ifdef VM
    delete coreMap;
    delete swap;
#ifdef REPL_AGING 
    delete replAging;
#elif REPL_OPTIMAL
    delete replOptimal;
#endif
#else
    delete bitMap;
#endif

    delete preemptionTimer;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    
    Exit(0);
}

