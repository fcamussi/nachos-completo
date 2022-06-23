#include "coremap.h"



CoreMap::CoreMap(int numPages)
{
    bitmap = new BitMap(numPages);
    spaceIds = new int[numPages];
    virtPages = new int[numPages];
}


CoreMap::~CoreMap()
{
    delete bitmap;
    delete [] spaceIds;
    delete [] virtPages;
}


int CoreMap::Find(int id, int virtPage)
{
    int phyPage = bitmap->Find();

    if (phyPage >= 0)
    {
        DEBUG('p', "Página reservada: phyPage %d, virtPage %d, id %d\n", phyPage, virtPage, id);
        spaceIds[phyPage] = id;
        virtPages[phyPage] = virtPage;
    }

    return phyPage;
}


void CoreMap::Clear(int phyPage)
{
    ASSERT(bitmap->Test(phyPage));
    DEBUG('p', "Página liberada: phyPage %d, virtPage %d, id %d\n", phyPage,
          virtPages[phyPage], spaceIds[phyPage]);
    spaceIds[phyPage] = -1;
    virtPages[phyPage] = -1;
    bitmap->Clear(phyPage);
}


int CoreMap::NumClear()
{
    return bitmap->NumClear();
}


bool CoreMap::Test(int phyPage)
{
    return bitmap->Test(phyPage);
}


int CoreMap::GetSpaceId(int phyPage)
{
    return spaceIds[phyPage];
}


int CoreMap::GetVirtualPage(int phyPage)
{
    return virtPages[phyPage];
}

