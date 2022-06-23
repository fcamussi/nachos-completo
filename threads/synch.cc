// synch.cc 
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks 
//      and condition variables (the implementation of the last two
//  are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"



//----------------------------------------------------------------------
// Semaphore::Semaphore
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    
    while (value == 0) {            // semaphore not available
    queue->Append(currentThread);       // so go to sleep
    currentThread->Sleep();
    } 
    value--;                    // semaphore available, 
                        // consume its value
    
    interrupt->SetLevel(oldLevel);      // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
    scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!



/* Constructor de Lock */
Lock::Lock(const char* debugName)
{
    name = debugName;
    sem = new Semaphore(debugName, 1);
    inher = new Semaphore(debugName, 1);
    thread = NULL;
}


/* Destructor de Lock */
Lock::~Lock()
{
    delete sem;
    delete inher;
}


/* Intenta adquirir el lock, si lo hace retorna de inmediato,
   sino, se duerme hasta poder hacerlo y luego retorna */
void Lock::Acquire()
{
    ASSERT(! isHeldByCurrentThread());
    DEBUG('l', "lock '%s': intentando adquirir por thread '%s'\n",
          getName(), currentThread->getName());
    if (thread) // si ya fue tomado por otro thread
    {
        // Herencia de prioridades: Asigno al thread que tiene adquirido el lock, el
        // máximo de las prioridades entre los threads que esperan para poder adquirirlo
        // y el thread que lo tiene adquirido
        if (currentThread->GetPriority() > thread->GetPriority())
        {
            DEBUG('l', "lock '%s': cambiando prioridad del thread '%s' a %d\n",
                  getName(), thread->getName(), currentThread->GetPriority());
            thread->SetPriority(currentThread->GetPriority());
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            if (thread->getStatus() == READY)
            {
                scheduler->ChangePriority(thread, currentThread->GetPriority());
            }
            interrupt->SetLevel(oldLevel);
        }
    }
    sem->P(); // tomo el semaforo
    oldPriority = currentThread->GetPriority(); // guardo la prioridad original
    thread = currentThread;
    DEBUG('l', "lock '%s': adquirido por thread '%s'\n", getName(), currentThread->getName());
}


/* Libera el Lock */
void Lock::Release()
{
    ASSERT(isHeldByCurrentThread());
    DEBUG('l', "lock '%s': liberado por thread '%s'\n", getName(), currentThread->getName());
    thread = NULL;
    DEBUG('l', "lock '%s': cambiando prioridad del thread '%s' a %d\n",
          getName(), currentThread->getName(), oldPriority);
    currentThread->SetPriority(oldPriority); // seteo la prioridad original
    sem->V(); // libero el semaforo
}


/* El current thread tiene tomado el Lock? */
bool Lock::isHeldByCurrentThread()
{
    return currentThread == thread;
}


/* Constructor de Condition */
Condition::Condition(const char* debugName, Lock* conditionLock)
{
    name = debugName;
    lock = conditionLock;
    queue = new List<Semaphore *>;
    helper = NULL;
    oldPriority = -1;
}


/* Destructor de Condition */
Condition::~Condition()
{
    delete queue;
}


/* Duerme hasta ser despertado por Signal */
void Condition::Wait()
{
    ASSERT(lock->isHeldByCurrentThread());
    ASSERT(currentThread != helper);
    Semaphore *sem = new Semaphore(name, 0);
    queue->Append(sem);
    if (helper) // si hay asignado un helper
    {
        // Herencia de prioridades: Asigno al helper (el que va a señalizar) 
        // el máximo de las prioridades entre los waiters y el helper
        if (oldPriority < 0) oldPriority = helper->GetPriority(); // guardo la original
        if (currentThread->GetPriority() > helper->GetPriority())
        {
            DEBUG('c', "condition '%s': cambiando prioridad del thread '%s' a %d\n",
                  getName(), helper->getName(), currentThread->GetPriority());
            helper->SetPriority(currentThread->GetPriority());
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            if (helper->getStatus() == READY)
            {
                scheduler->ChangePriority(helper, currentThread->GetPriority());
            }
            interrupt->SetLevel(oldLevel);
        }
    }
    DEBUG('c', "condition '%s': durmiendo en thread '%s' y esperando Signal\n",
          getName(), currentThread->getName());
    lock->Release(); // no es atómico, el llamante
    sem->P();        // tiene que rechequear la condición
    DEBUG('c', "condition '%s': signal recibida y despertando en thread '%s'\n",
          getName(), currentThread->getName());
    delete sem;
    lock->Acquire();
}


/* Despierta un thread dormido por Wait */
void Condition::Signal()
{
    ASSERT(lock->isHeldByCurrentThread());
    Semaphore *sem = queue->Remove();
	lock->Release();
    if (sem != NULL)
    {
        DEBUG('c', "condition '%s': signal enviada desde thread '%s'\n",
              getName(), currentThread->getName());
        sem->V();
    }
    else
    {
        // Debug muy útil!!!
        DEBUG('c', "condition '%s': signal enviada y perdida desde thread '%s'\n",
              getName(), currentThread->getName());
    }
    lock->Acquire();
    if (helper && queue->IsEmpty()) // Si hay asignado un helper y ya no hay waiters
    {
        // Establezco al helper su prioridad original
        DEBUG('c', "condition '%s': cambiando prioridad del thread '%s' a %d\n",
               getName(), helper->getName(), oldPriority);
        helper->SetPriority(oldPriority);
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        if (helper->getStatus() == READY)
        {
            scheduler->ChangePriority(helper, oldPriority);
        }
        interrupt->SetLevel(oldLevel);
    }
}


/* Despierta todos los threads dormidos */
void Condition::Broadcast()
{
    while (! queue->IsEmpty())
    {
        Signal();
    }
}


/* Setea el helper (el thread que va a hacer signal), así cuando se
   haga wait se aplica herencia de prioridad sobre el mismo */
void Condition::SetHelper(Thread *thread)
{
    ASSERT(lock->isHeldByCurrentThread());
    DEBUG('c', "condition '%s': seteando como helper al thread '%s'\n",
          getName(), thread->getName());
    helper = thread;
}


/* Constructor de Port */
Port::Port(const char* debugName, int port)
{
    name = debugName;
    Port::port = port;
    readyToSend = 0;
    readyToReceive = 0;
    sent = 0;
    received = 0;
    buffer = NULL;
    lock = new Lock(debugName);
    send = new Condition(debugName, lock);
    receive = new Condition(debugName, lock);
    busyToSend = new Lock(debugName);
    busyToReceive = new Lock(debugName);
}


/* Destructor de Port */
Port::~Port()
{
    delete send;
    delete receive;
    delete lock;
    delete busyToSend;
    delete busyToReceive;
}


/* Sincroniza con el receptor y copia el mensaje en el buffer del mismo */
void Port::Send(int message)
{
    busyToSend->Acquire(); // tomo el puerto para enviar
    readyToSend = 1; // indico que estoy listo para enviar
    lock->Acquire(); // adquiero el lock asociado a las variables de condición
    receive->Signal(); // despierto al receptor si está dormido
    while (readyToReceive == 0) // espero a que el receptor esté listo para recibir
        send->Wait();
    readyToReceive = 0; // Reseteo la variable
    *buffer = message; // Escribo en el buffer proporcionado por Receive
    sent = 1; // indico que el mensaje ya fue enviado
    DEBUG('m', "message '%s': mensaje '%d' escrito en puerto '%d' en thread '%s'\n",
          getName(), message, port, currentThread->getName());
    receive->Signal(); // despierto por si el receptor está dormido
    while (received == 0) // espero a que el receptor haya recibido el mensaje
        send->Wait();
    received = 0; // Reseteo la variable
    lock->Release(); // libero el lock asociado a las variables de condición
    busyToSend->Release(); // libero el puerto para enviar
}


/* Sincroniza con el emisor y recibe el mensaje enviado por el mismo */
void Port::Receive(int *message)
{
    busyToReceive->Acquire(); // tomo el puerto para recibir
    buffer = message; // guardo en buffer el buffer del receptor
    readyToReceive = 1; // indico que estoy listo para recibir
    lock->Acquire(); // adquiero el lock asociado a las variables de condición
    send->Signal(); // despierto al emisor si está dormido
    while (readyToSend == 0) // espero a que el emisor esté listo para enviar
        receive->Wait();
    readyToSend = 0; // Reseteo la variable
    while (sent == 0) // espero a que el emisor haya enviado el mensaje
        receive->Wait();
    sent = 0; // Reseteo la variable
    received = 1; // indico que el mensaje fue recibido
    send->Signal(); // despierto al emisor si está dormido
    DEBUG('m', "message '%s': mensaje '%d' recibido en puerto '%d' en thread '%s'\n",
          getName(), *message, port, currentThread->getName());
    lock->Release(); // libero el lock asociado a las variables de condición
    busyToReceive->Release(); // libero el puerto para recibir
}



/* Constructor de Message */
Message::Message(const char* debugName, int nPorts)
{
    name = debugName;
    Message::nPorts = nPorts;
    lock = new Lock(debugName);
    ports = new Port *[nPorts]; // array de punteros a Port
    for (int c = 0; c < nPorts; c++) // seteo a NULL los punteros
    {
        ports[c] = NULL;
    }
}


/* Destructor de Message */
Message::~Message()
{
    for (int c = 0; c < nPorts; c++) // Libero todos los puertos reservados
    {
        if (ports[c] != NULL) delete ports[c];
    }
    delete [] ports;
    delete lock;
}


/* Envío el mensaje message al puerto port */
void Message::Send(int port, int message)
{
    Port *p = GetPort(port); // obtengo el puerto port

    p->Send(message);
}


/* Recibo un mensaje por el puerto port y lo almaceno en message */
void Message::Receive(int port, int *message)
{
    Port *p = GetPort(port); // obtengo el puerto port

    p->Receive(message);
}


/* Obtengo un puerto del array de puertos, si todavia no se alojó ninguno, lo hago */
Port *Message::GetPort(int port)
{
    ASSERT(port >= 0 && port < nPorts);
    lock->Acquire(); // Me aseguro que se haga un solo new para el mismo puerto
    if (ports[port] == NULL) // Si es NULL entonces creo un puerto nuevo
    {
        ports[port] = new Port(getName(), port);
    }
    lock->Release();
    return ports[port]; // lo retorno
}


