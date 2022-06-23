#ifndef COREMAP_H
#define COREMAP_H

#include "bitmap.h"


class CoreMap
{
  public:
    CoreMap(int numPages);
    ~CoreMap();
    int Find(int id, int virtPage);
    void Clear(int phyPage);
    int NumClear();
    bool Test(int phyPage);
    int GetSpaceId(int phyPage);
    int GetVirtualPage(int phyPage);
  private:
    BitMap *bitmap;
    int *spaceIds;
    int *virtPages;
};


#endif // COREMAP_H

