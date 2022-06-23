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


Thread *thread[4];
Lock *lock = new Lock("lock");



void func0(void *arg)
{
    lock->Acquire();
    currentThread->Yield();
    lock->Release();
}


void func1(void *arg)
{
    while(true)
    {
        currentThread->Yield();
    }
}


void func2(void *arg)
{
    lock->Acquire();
    lock->Release();
    printf("Perfecto!!!\n");
    exit(0);
}


void func3(void *arg)
{
    thread[0] = new Thread("thread0", false, 0); // prioridad baja
    thread[1] = new Thread("thread1", false, 1); // prioridad intermedia
    thread[2] = new Thread("thread2", false, 2); // prioridad alta
    thread[0]->Fork(func0, NULL);
    currentThread->Yield();
    thread[1]->Fork(func1, NULL);
    thread[2]->Fork(func2, NULL);
}


void ThreadTest()
{
    thread[3] = new Thread("thread3", false, 9);
    thread[3]->Fork(func3, NULL);
}

