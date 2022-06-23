#ifndef SWAP_H
#define SWAP_H

#include "filesys.h"



class SwapFile
{
  public:
    SwapFile(int id, int pages);
    ~SwapFile();
    void WritePage(const char *buffer, int page);
    void ReadPage(char *buffer, int page);
  private:
    int pages;
    int count;
    int *locations;
    char *filename;
    OpenFile *of;
};


class Swap
{
  public:
    Swap(int maxId);
    ~Swap();
    void AddSpace(int id, int pages);
    void RemoveSpace(int id);
    void WritePage(int id, const char *buffer, int page);
    void ReadPage(int id, char *buffer, int page);
  private:
    int maxId;
    SwapFile **swapFiles;
};


#endif // SWAP_H

