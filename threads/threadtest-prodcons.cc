// threadtest.cc 
//  Simple test case for the threads assignment.
//
//  Create several threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield, 
//  to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include <time.h>

#define N 10


static int buf[N];
static int i = 0;
Lock *lock;
Condition *vacio;
Condition *lleno;


/* Productor */
void productor(void *arg)
{
    for ( ; ; )
    {
        int item = rand() % 10;
        printf("Producimos %d\n", item);
        fflush(stdout);
        lock->Acquire();
        while (i >= N)
            lleno->Wait();
        buf[i] = item;
        i++;
        vacio->Signal();
        lock->Release();
    }
}


/* Consumidor */
void consumidor(void *arg)
{
    for ( ; ; )
    {
        int item;
        lock->Acquire();
        while (i <= 0)
            vacio->Wait();
        item = buf[i - 1];
        i--;
        printf("Consumo %d\n", item);
        fflush(stdout);
        lleno->Signal();
        lock->Release();
    }
}


void ThreadTest()
{
    srand(time(NULL)); 

    lock = new Lock("lock");
    vacio = new Condition("vacio", lock);
    lleno = new Condition("lleno", lock);

    Thread *prodThread = new Thread("Productor", false, 0);
    Thread *consThread = new Thread("Consumidor", false, 0);
    prodThread->Fork(productor, NULL);
    consThread->Fork(consumidor, NULL);
}

