#ifndef EXCEPTION_H
#define EXCEPTION_H


void ExceptionHandler(ExceptionType which);
void StartProcess(const char *filename, Arguments *arguments);


static void SysCall();

static void SysCallHalt();
static void SysCallExit();
static void SysCallExec();
static void SysCallJoin();
static void SysCallCreate();
static void SysCallOpen();
static void SysCallRead();
static void SysCallWrite();
static void SysCallClose();
static void SysListProc();

static void IncrementPC();
static void Fork(void *arg);



#endif // EXCEPTION_H

