#include "paging.h"
#include "system.h"
#include "swap.h"
#include "replacement.h"



int tlbPos; // posición de la entrada siguiente a ser usada en el TLB


void SetTLBPos(int pos)
{
    tlbPos = pos;
}



/* Manejador de fallo de página */
void PageFaultHandler()
{
    AddrSpace *space = currentThread->space;
    TranslationEntry *pageTable = space->GetPageTable();
    int id = currentThread->GetId();
    int virtAddr = machine->ReadRegister(BadVAddrReg);
    int virtPage = virtAddr / PageSize;

    if (virtPage >= space->GetNumPages())
    {
        printf("Violación de segmento. Finalizando el proceso %s...\n",
               currentThread->getName());
        currentThread->Finish();
        return;
    }

#ifdef REPL_AGING
    replAging->Update(); // Actualizo los contadores del algoritmo de envejecimiento
#endif

    DEBUG('p', "Excepcion 'PageFaultException' recibida. BadVAddrReg: %d, virtPage: %d, id: %d\n",
          virtAddr, virtPage, id);

    // La página está en memoria?
    if (! pageTable[virtPage].valid) // No. Hay que cargarla
    {
        if (coreMap->NumClear() < 1) // Hay páginas físicas disponibles?
        { // No, Hay que liberar una página (que puede ser de cualquier proceso)
            int relPhyPage; // Página física a liberar
#ifdef REPL_AGING 
            relPhyPage = replAging->GetRelPage(); // algoritmo de remplazo aging
#elif REPL_OPTIMAL
            relPhyPage = replOptimal->GetRelPage(); // algoritmo de remplazo óptimo
#else
            relPhyPage = ReplSimple(); // algoritmo de remplazo simple
#endif
            int relVirtPage = coreMap->GetVirtualPage(relPhyPage);
            int relId = coreMap->GetSpaceId(relPhyPage);
            AddrSpace *relSpace = processTable->GetProcess(relId)->space;
            TranslationEntry *relPageTable = relSpace->GetPageTable();

            // Si una página está sucia hay que guardarla en la SWAP
            bool dirty = false;
            if (relPageTable[relVirtPage].dirty) // Está marcada como sucia en el page table?
            { // Sí
                dirty = true;
            }
            else // No, pero puede estar marcada como sucia en el TLB
            {
                for (int c = 0; c < TLBSize; c++)
                {
                    if (machine->tlb[c].physicalPage == relPhyPage &&
                        machine->tlb[c].valid && machine->tlb[c].dirty)
                    { // Está marcada como sucia en el TLB
                        relPageTable[relVirtPage].dirty = true; // actualizo el page table
                        dirty = true;
                        break;
                    }
                }
            }
            if (dirty)
            { // Está sucia, hay que guardarla en la SWAP
                swap->WritePage(relId, &machine->mainMemory[relPhyPage * PageSize], relVirtPage);
            }

            // Se libera de la memoria y se marca como inválida en el page table
            coreMap->Clear(relPhyPage);
            relPageTable[relVirtPage].valid = false;

            // Además si la página que se libera está en el TLB como válida hay que
            // marcarla como inválida
            int c;
            for (c = 0; c < TLBSize; c++)
            {
                if (machine->tlb[c].physicalPage == relPhyPage && machine->tlb[c].valid)
                {
                    machine->tlb[c].valid = false;
                    break;
                }
            }
        }

        // Se carga la página en memoria
        int phyPage = coreMap->Find(id, virtPage); // pido una página al core map
        pageTable[virtPage].physicalPage = phyPage;
        // Si una página inválida está sucia entonces tiene que haber sido guardada
        // en la swap previamente
        if (pageTable[virtPage].dirty) // Está sucia, la página se carga desde la swap
        {
            swap->ReadPage(id, &machine->mainMemory[phyPage * PageSize], virtPage);
        }
        else // No está sucia, cargamos la página directamente desde el ejecutable
        {
            space->LoadPage(virtPage);
        }
        pageTable[virtPage].valid = true;
        stats->numHardPageFaults++; // estadistica para fallo duro de pagina
    }

    // Actualizamos el TLB
    if (machine->tlb[tlbPos].valid) // Está siendo utilizada
    {
        // Guardo el estado actual
        pageTable[machine->tlb[tlbPos].virtualPage] = machine->tlb[tlbPos];
    }
    DEBUG('p', "Se actualiza el TLB en la posición %d\n", tlbPos);
    machine->tlb[tlbPos] = pageTable[virtPage];
    tlbPos = (tlbPos + 1) % TLBSize; // siguiente posición (se va rotando)
}


