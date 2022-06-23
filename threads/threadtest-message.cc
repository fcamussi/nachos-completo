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



Message *message = new Message("message", 256);



void func1(void *arg)
{
    message->Send(2, 5);
}

void func2(void *arg)
{
    int a;
    message->Receive(2, &a);
    printf("%d\n", a);
}

void func3(void *arg)
{
    int a;
    message->Receive(1, &a);
    printf("%d\n", a);
}

void func4(void *arg)
{
    message->Send(1, 7);
}

void func5(void *arg)
{
    message->Send(1, 11);
}

void func6(void *arg)
{
    message->Send(1, 14);
}

void func7(void *arg)
{
    int a;
    message->Receive(1, &a);
    printf("%d\n", a);
}

void func8(void *arg)
{
    int a;
    message->Receive(1, &a);
    printf("%d\n", a);
}



void ThreadTest()
{
    Thread *thread1 = new Thread("thread1", true, 0);
    Thread *thread2 = new Thread("thread2", true, 0);
    Thread *thread3 = new Thread("thread3", true, 0);
    Thread *thread4 = new Thread("thread4", true, 0);
    Thread *thread5 = new Thread("thread5", true, 0);
    Thread *thread6 = new Thread("thread6", true, 0);
    Thread *thread7 = new Thread("thread7", true, 0);
    Thread *thread8 = new Thread("thread8", true, 0);
    thread1->Fork(func1, NULL);
    thread2->Fork(func2, NULL);
    thread3->Fork(func3, NULL);
    thread4->Fork(func4, NULL);
    thread5->Fork(func5, NULL);
    thread6->Fork(func6, NULL);
    thread7->Fork(func7, NULL);
    thread8->Fork(func8, NULL);
    thread1->Join();
    thread2->Join();
    thread3->Join();
    thread4->Join();
    thread5->Join();
    thread6->Join();
    thread7->Join();
    thread8->Join();
}

