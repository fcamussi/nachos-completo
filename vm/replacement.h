#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include "filesys.h"
#include "list.h"

#define COUNTER_SIZE 32 // Tiene que ser menor a sizeof(unsigned long long)



int ReplSimple();



typedef struct {
    unsigned long long bits : COUNTER_SIZE;
} Counter;


class ReplAging
{
  public:
    ReplAging(int numPages);
    ~ReplAging();
    void Update();
    int GetRelPage();
    void Print();
  private:
    int numPages;
    Counter *counters;
};



class ReplOptimal
{
  public:
    ReplOptimal(const char *pageRefs);
    ~ReplOptimal();
    int GetRelPage();
  private:
    void GenList();
    bool IsInTable(int page);
    int numVirtPages;
    int *table;
    int *label;
    OpenFile *of;
    int numRefs;
    int *refs;
    List<int> *optRefs;
};


#endif // REPLACEMENT_H

