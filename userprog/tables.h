#ifndef TABLE_H
#define TABLE_H

#include "syscall.h"
#include "filesys.h"

#define MAX_OPENFILES 16
#define MAX_PROCESSES 256


class Thread;



/* Clase Table */
template <class Item>
class Table
{
    public:
        Table(int entries);
        ~Table();
        int Add(Item *item);
        Item *Get(int n);
        void Remove(int n);
        bool IsFull();
        bool IsInUse(int n);
        void Free();
    private:
        Item **table;
        int entries;
        int freeEntries;
};


/* Constructor */
template <class Item>
Table<Item>::Table(int entries)
{
    table = new Item *[entries];
    for (int c = 0; c < entries; c++)
        table[c] = NULL;
    Table::entries = entries;
    freeEntries = entries;
}


/* Destructor */
template <class Item>
Table<Item>::~Table()
{
    delete[] table;
}


/* Libera todos los items */
template <class Item>
void Table<Item>::Free()
{
    for (int c = 0; c < entries; c++)
        if (table[c]) delete table[c];
    freeEntries = entries;
}


/* Agrega un item en la primer posición libre */
template <class Item>
int Table<Item>::Add(Item *item)
{
    int n = 0;

    while (table[n]) n++; // busco la primer entrada libre
    table[n] = item;
    freeEntries--;
    return n;
}


/* Obtiene el item de la posición n de la tabla */
template <class Item>
Item *Table<Item>::Get(int n)
{
    return table[n];
}


/* Remueve de la tabla el item de la posición n */
template <class Item>
void Table<Item>::Remove(int n)
{
    table[n] = NULL;
    freeEntries++;
}


/* Comprueba si la tabla está llena */
template <class Item>
bool Table<Item>::IsFull()
{
    if (freeEntries > 0)
        return false;
    else
        return true;
}


/* Comprueba si la posición n está ocupada */
template <class Item>
bool Table<Item>::IsInUse(int n)
{
    if (n < 0) return false;
    else return (table[n] ? true : false);
}



/* Clase utilizada para almacenar los ficheros abiertos */
class OpenFileTable
{
    public:
        OpenFileTable(int entries) { table = new Table<OpenFile>(entries); }
        ~OpenFileTable() { table->Free(), delete table; } // Cierra los archivos abiertos y se libera
        OpenFileId AddOpenFile(OpenFile *of) { return table->Add(of) + 2; } // ConsoleInput = 0 y ConsoleOutput = 1
        OpenFile *GetOpenFile(OpenFileId id) { return table->Get(id - 2); }
        void RemoveOpenFile(OpenFileId id) { table->Remove(id - 2); }
        bool IsFull() { return table->IsFull(); }
        bool IsInUse(OpenFileId id) { return table->IsInUse(id - 2); }
    private:
        Table<OpenFile> *table;
};



/* Clase utilizada para almacenar los procesos creados */
class ProcessTable
{
    public:
        ProcessTable(int entries) { table = new Table<Thread>(entries); }
        ~ProcessTable() { delete table; }
        SpaceId AddProcess(Thread *thread) { return table->Add(thread); }
        Thread *GetProcess(SpaceId id) { return table->Get(id); }
        void RemoveProcess(SpaceId id) { table->Remove(id); }
        bool IsFull() { return table->IsFull(); }
        bool IsInUse(SpaceId id) { return table->IsInUse(id); }
    private:
        Table<Thread> *table;
};



#endif // TABLE_H
