#include "replacement.h"
#include "system.h"




//----------------------------------------------------------------------
// Algoritmo de reemplazo simple
//----------------------------------------------------------------------

int ReplSimple()
{
    static int page = 0;
    int tmp = page;

    ASSERT(coreMap->NumClear() < 1); // No tiene que haber páginas físicas libres

    page = (page + 1) % NumPhysPages;
    return tmp;
}



//----------------------------------------------------------------------
// Clase que implementa el algoritmo de reemplazo conocido como
// envejecimiento (aproximación a LRU)
//----------------------------------------------------------------------

ReplAging::ReplAging(int numPages)
{
    ReplAging::numPages = numPages;
    counters = new Counter[numPages];

    for (int c = 0; c < numPages; c++)
    {
        counters[c].bits = 0;
    }
}


ReplAging::~ReplAging()
{
    delete [] counters;
}


/* Actualiza los contadores en base al bit use del TLB */
void ReplAging::Update()
{
    int page;
    int t;

    for (page = 0; page < numPages; page++)
    {
        // Desplazo los bits del contador un lugar hacia la derecha y coloco un uno
        // adelante si la página está en uso, o un cero si no lo está
        // (Estoy asumiendo que el procesador anfitrión es little endian)
        for (t = 0; t < TLBSize; t++)
        {
            if (page == machine->tlb[t].physicalPage && machine->tlb[t].valid)
            {
                counters[page].bits = (counters[page].bits >> 1)
                                    | (unsigned long long)machine->tlb[t].use << (COUNTER_SIZE - 1);
                machine->tlb[t].use = false; // desactivo el bit use nuevamente (si fue activado)
                break;
            }
        }
        if (t == TLBSize) // la página no está en el TLB, por lo tanto no está en uso
        {
            counters[page].bits = (counters[page].bits >> 1) | 0ULL << (COUNTER_SIZE - 1);
        }
    }
}


/* Retorna la página que hace más tiempo no se usa */
int ReplAging::GetRelPage()
{
    int menor;

    ASSERT(coreMap->NumClear() < 1); // No tiene que haber páginas físicas libres

    menor = 0;
    for (int c = 1; c < numPages; c++)
    {
        if (counters[c].bits < counters[menor].bits)
        {
            menor = c;
        }
    }

    return menor;
}


void ReplAging::Print()
{
    for (int n = 0; n < numPages; n++)
    {
        for (int c = COUNTER_SIZE - 1; c >= 0 ; c--)
        {    
            printf("%llu", (counters[n].bits >> c) & 1);
        }
        printf("\n");
    }
}



//----------------------------------------------------------------------
// Clase que implementa el algoritmo de reemplazo óptimo
//----------------------------------------------------------------------

ReplOptimal::ReplOptimal(const char *pageRefs)
{
    char buffer[11];
    int offset = 30; // encabezado del archivo

    of = fileSystem->Open(pageRefs);
    ASSERT(of);
    ASSERT(of->ReadAt(buffer, 11, 7) == 11);
    numVirtPages = atoi(buffer); // cantidad de páginas que usa el programa
    table = new int[NumPhysPages]; // tabla indexada por páginas física con las páginas virtuales
    label = new int[NumPhysPages]; // etiquetas con la posición de la próxima referencia
    for (int c = 0; c < NumPhysPages; c++)
    {
        table[c] = -1; // entrada vacía
    }
    numRefs = (of->Length() - offset) / sizeof(buffer); // cantidad de referencias
    refs = new int[numRefs]; // referencias que se leen desde el archivo
    for (int c = 0; c < numRefs; c++)
    {
        ASSERT(of->ReadAt(buffer, 11, offset + 11 * c) == 11);
        refs[c] = atoi(buffer);
    }
    optRefs = new List<int>; // acá se cargarán las referencias en orden óptimo
    GenList();
}


ReplOptimal::~ReplOptimal()
{
    delete of;
    delete table;
    delete label;
    delete refs;
    delete optRefs;
}


int ReplOptimal::GetRelPage()
{
    ASSERT(coreMap->NumClear() < 1); // No tiene que haber páginas físicas libres

    return optRefs->Remove();
}


void ReplOptimal::GenList()
{
    int t = 0;
    int c = 0;

    ASSERT(numVirtPages > NumPhysPages); // No tiene sentido sino

    // cargo las primeras hasta llenar la tabla
    // (las que serían cargadas en memoria hasta llenarla)
    while (t < NumPhysPages)
    {
        if (! IsInTable(refs[c]))
        {
            table[t] = refs[c];
            t++;
        }
        c++;
    }

    while (c < numRefs)
    {
        // busco la página que hay que alojar
        int virtPageIn; // pagina que se va a alojar
        do
        {
            c++;
        } while (IsInTable(refs[c]));
        virtPageIn = refs[c];

        // etiqueto las páginas que están en la tabla con la posición
        // en el fichero de referencias
        for (t = 0; t < NumPhysPages; t++)
        {
            label[t] = numRefs; // ultima posición + 1
            for (int n = c; n < numRefs; n++)
            {
                if (table[t] == refs[n])
                {
                    label[t] = n - c + 1; // cuantas posiciones faltan para que se referencie
                    break; // salgo del bucle porque quiero la primera
                }
            }
        }
        // elijo para liberar la página con la mayor etiqueta
        int pageOut = 0; // pagina que se va a desalojar
        for (t = 1; t < NumPhysPages; t++)
        {
            if (label[t] > label[pageOut])
            {
                pageOut = t;
            }
        }
        optRefs->Append(pageOut); // la agrego a la lista
        table[pageOut] = virtPageIn; // la reemplazo en la tabla del algoritmo
    }
    c++;
}


bool ReplOptimal::IsInTable(int page)
{
    for (int c = 0; c < NumPhysPages; c++)
    {
        if (table[c] == page) return true;
    }
    return false;
}

