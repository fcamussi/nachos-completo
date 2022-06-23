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



int a = 0;



void func1(void *arg)
{
    for (int c = 0 ; c < 20 ; c++)
	{
		int tmp = a;
		tmp++;
		if (c == 0) currentThread->Yield();
		a = tmp;
		if (c == 0) currentThread->Yield();
	}
}

void func2(void *arg)
{
    for (int c = 0 ; c < 20 ; c++)
	{
		int tmp = a;
		if (c == 19) currentThread->Yield();
		tmp++;
 		a = tmp;
		if (c == 18) currentThread->Yield();
	}
}



void ThreadTest()
{
    Thread *thread1 = new Thread("thread1", true, 0);
    Thread *thread2 = new Thread("thread2", true, 0);
    thread1->Fork(func1, NULL);
    thread2->Fork(func2, NULL);
    thread1->Join();
    thread2->Join();
	printf("%d\n", a);
}

