// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"
#include "copy.h"


#define UserStackSize		1024 	// increase this as necessary!


void SwapHeader (NoffHeader *noffH);


class AddrSpace {
  public:
    AddrSpace(unsigned int numPages, OpenFile *executable, NoffHeader noffH);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
    void LoadProgram();
    void LoadArguments(Arguments *arguments);
    void LoadPage(int virtualPage);
    TranslationEntry *GetPageTable() { return pageTable; }
    int GetNumPages() { return numPages; }

  private:
    int Translate(int virtualAddr);
    void LoadPageSegment(int virtualPage, int virtualAddr, int inFileAddr, int size, bool zero, bool ro);
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    OpenFile *executable;
    NoffHeader noffH;
#ifdef VM
    int id; // id del proceso
    TranslationEntry *tlb; // copia local del TLB
#endif
};

#endif // ADDRSPACE_H
