#include "system.h"
#include "swap.h"



SwapFile::SwapFile(int id, int pages)
{
    SwapFile::pages = pages;
    count = 0;
    locations = new int[pages]; // indica en que bloque esta cada pagina
    filename = new char[100];

    for (int page = 0; page < pages; page++) locations[page] = -1; // no usadas
    sprintf(filename, "SWAP.%d", id);
    fileSystem->Create(filename, 0); // vacío al principio
    of = fileSystem->Open(filename);
    ASSERT(of);
}


SwapFile::~SwapFile()
{
    delete [] locations;
    fileSystem->Remove(filename);
    delete [] filename;
    delete of;
}


void SwapFile::WritePage(const char *buffer, int page)
{
    int written;

    ASSERT(page >= 0 && page < pages);
    if (locations[page] < 0) // no asignada
    {
        locations[page] = count; // se asigna
        count++;
    }
    written = of->WriteAt(buffer, PageSize, PageSize * locations[page]);
    ASSERT(written == PageSize);
}


void SwapFile::ReadPage(char *buffer, int page)
{
    int read;

    ASSERT(page >= 0 && page < pages);
    ASSERT(locations[page] >= 0);
    read = of->ReadAt(buffer, PageSize, PageSize * locations[page]);
    ASSERT(read == PageSize);
}



Swap::Swap(int maxId)
{
    Swap::maxId = maxId;
    swapFiles = new SwapFile *[maxId];
    for (int id = 0; id < maxId; id++) swapFiles[id] = NULL;
}


Swap::~Swap()
{
    for (int id = 0; id < maxId; id++)
    {
        if (swapFiles[id]) delete swapFiles[id];
    }
    delete [] swapFiles;
}


void Swap::AddSpace(int id, int pages)
{
    ASSERT(id >= 0 && id < maxId);
    DEBUG('w', "Se crea un SWAP: id %d, pages %d\n", id, pages);
    swapFiles[id] = new SwapFile(id, pages);
}


void Swap::RemoveSpace(int id)
{
    ASSERT(id >= 0 && id < maxId);
    DEBUG('w', "Se elimina un SWAP: id %d\n", id);
    delete swapFiles[id];
    swapFiles[id] = NULL;
}


void Swap::WritePage(int id, const char *buffer, int page)
{
    ASSERT(id >= 0 && id < maxId);
    ASSERT(swapFiles[id]);
    DEBUG('w', "Se escribe en SWAP: id %d, page %d\n", id, page);
    swapFiles[id]->WritePage(buffer, page);
}


void Swap::ReadPage(int id, char *buffer, int page)
{
    ASSERT(id >= 0 && id < maxId);
    ASSERT(swapFiles[id]);
    DEBUG('w', "Se lee de SWAP: id %d, page %d\n", id, page);
    swapFiles[id]->ReadPage(buffer, page);
}

