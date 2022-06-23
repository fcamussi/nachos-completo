// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"

#ifdef VM
#include "paging.h"
#endif


//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(unsigned int numPages, OpenFile *executable, NoffHeader noffH)
{
    unsigned int i;

    DEBUG('z', "Initializing address space, num pages %d, size %d\n",
          numPages, numPages * PageSize);

    // first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++)
    {
	    pageTable[i].virtualPage = i;
#ifdef VM
	    pageTable[i].physicalPage = -1;
	    pageTable[i].valid = false;
#else
	    pageTable[i].physicalPage = bitMap->Find();
	    pageTable[i].valid = true;
#endif
	    pageTable[i].use = false;
	    pageTable[i].dirty = false;
        // if the code segment was entirely on a separate page,
        // we could set its pages to be read-only
	    pageTable[i].readOnly = false;
    }
    AddrSpace::numPages = numPages;
    AddrSpace::executable = executable;
    AddrSpace::noffH = noffH;
#ifdef VM
    id = currentThread->GetId();
    swap->AddSpace(id, numPages);

    // inicializamos la copia local del TLB
    tlb = new TranslationEntry[TLBSize];
    for (int c = 0; c < TLBSize; c++)
    {
        tlb[c].valid = false;
    }
#endif
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    unsigned int i;

    for (i = 0; i < numPages; i++)
    {
#ifdef VM
        if (pageTable[i].valid) coreMap->Clear(pageTable[i].physicalPage);
#else
        bitMap->Clear(pageTable[i].physicalPage);
#endif
    }
    delete[] pageTable;
    delete executable; // close file
#ifdef VM
    swap->RemoveSpace(id);
    delete[] tlb;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('z', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
#ifdef VM
    // Guardo el estado actual del TLB y actualizo los dirty bits 
    // en el page table local
    for (int c = 0; c < TLBSize; c++)
    {
        tlb[c] = machine->tlb[c];
        if (machine->tlb[c].valid && machine->tlb[c].dirty)
        {
            pageTable[machine->tlb[c].virtualPage].dirty = true;
        }
    }
#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifdef VM
    // Cargo en el TLB las páginas que estaban antes pero que son validas
    int tlbCount = 0;

    for (int c = 0; c < TLBSize; c++)
    {
        if (tlb[c].valid && pageTable[tlb[c].virtualPage].valid)
        {
            machine->tlb[tlbCount] = tlb[c];
            tlbCount++;
        }
    }
    // Marco las restantes como invalidas
    for (int c = tlbCount; c < TLBSize; c++)
    {
        machine->tlb[c].valid = false;
    }

    SetTLBPos(tlbCount % TLBSize); // Seteo la posicion siguiente a usar del TLB
#else
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}

//----------------------------------------------------------------------

void AddrSpace::LoadProgram() // Desde el ejecutable
{
    unsigned i;

    for (i = 0; i < numPages; i++)
    {
	    LoadPage(pageTable[i].virtualPage);
    }
}

//----------------------------------------------------------------------

void AddrSpace::LoadArguments(Arguments *arguments)
{
    // La memoria crece hacia arriba y el stack hacia abajo
    // Por lo tanto, el segmento de código empieza en la posición 0,
    // seguido por los segmentos de datos inicializados y sin inicializar
    // y la pila comienza en PageSize * numPage - 1 (y crece hacia abajo)

    int sp = machine->ReadRegister(StackReg); // stack pointer

    /* sp en este momento apunta al tope de la pila
       (o sea, al fondo de la memoria, bueno, casi al fondo) */

    sp = arguments->CopyToUserStack(sp); // pongo los argumentos en el stack

    machine->WriteRegister(4, arguments->GetArgc()); // primer parametro de main (int argc)
    machine->WriteRegister(5, sp); // segundo parametro de main (char *argv[])

    machine->WriteRegister(StackReg, sp - 8); // ajusto el sp

    DEBUG('z', "Argumentos copiados a la pila, se modifica el stack pointer a %d\n", sp - 8);
}

//----------------------------------------------------------------------
// Carga una pagina completamente
//----------------------------------------------------------------------

void AddrSpace::LoadPage(int virtualPage)
{
    int stackVirtualAddr = PageSize * numPages - UserStackSize;

    LoadPageSegment(virtualPage, noffH.code.virtualAddr, noffH.code.inFileAddr,
                    noffH.code.size, false, true); // code
    LoadPageSegment(virtualPage, noffH.initData.virtualAddr, noffH.initData.inFileAddr,
                    noffH.initData.size, false, false); // data
    LoadPageSegment(virtualPage, noffH.uninitData.virtualAddr, 0,
                    noffH.uninitData.size, true, false); // uninitializate code
    LoadPageSegment(virtualPage, stackVirtualAddr, 0, UserStackSize, true, false); // stack
}

//----------------------------------------------------------------------
// Hace la traduccion a direccion fisica sin provocar fallos de pagina
// como hace machine->Translate(...)
//----------------------------------------------------------------------
int AddrSpace::Translate(int virtualAddr)
{
    int virtualPage = virtualAddr / PageSize;
    int addr = virtualAddr % PageSize;
    return pageTable[virtualPage].physicalPage * PageSize + addr;
}

//----------------------------------------------------------------------
// Carga una pagina con la parte del segmento que le corresponde
// y que se pasa como parametro
// Si el parametro ro es true y se usa completamente entonces se marca
// como de solo lectura
//----------------------------------------------------------------------
void AddrSpace::LoadPageSegment(int virtualPage, int virtualAddr, int inFileAddr,
                                int size, bool zero, bool ro)
{
    int pageBegin = virtualPage * PageSize;
    int pageEnd = (virtualPage + 1) * PageSize - 1;
    int begin = virtualAddr;
    int end = virtualAddr + size - 1;

    if (size <= 0) return;
    
    if (begin >= pageBegin && begin <= pageEnd)
    { // El segmento comienza dentro de la pagina virtualPage
        int s1 = end - begin + 1;
        int s2 = pageEnd - begin + 1;
        virtualAddr = begin;
        size = s1 < s2 ? s1 : s2;
    }
    else if (begin < pageBegin && end >= pageBegin)
    { // El segmento comienza antes de la pagina virtualPage
        int s = end - pageBegin + 1;
        int a = pageBegin - begin;
        virtualAddr = pageBegin;
        inFileAddr += a;
        size = s < PageSize ? s : PageSize;
    }
    else
    { // El segmento comienza después de la pagina virtualPage
        return;
    }

    // La página se usa completamente
    if (size == PageSize) pageTable[virtualPage].readOnly = ro;

    if (zero) // Ponemos ceros
    {
        DEBUG('p', "Inicializando con ceros: virtualPage %d, virtualAddr %d, size %d, ro: %d\n",
              virtualPage, virtualAddr, size, pageTable[virtualPage].readOnly);
        bzero(&machine->mainMemory[Translate(virtualAddr)], size);
    }
    else // Sino, ponemos datos desde el fichero
    {
        DEBUG('p', "Inicializando con datos: virtualPage %d, virtualAddr %d, size %d, ro: %d\n",
              virtualPage, virtualAddr, size, pageTable[virtualPage].readOnly);
        executable->ReadAt(&machine->mainMemory[Translate(virtualAddr)], size, inFileAddr);
    }
}

