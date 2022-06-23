#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include "list.h"



/* Clase PriorityQueue */
template <class Item>
class PriorityQueue {
  public:
    PriorityQueue(int numQueues);
    ~PriorityQueue();
    void Append(Item item, int queue);
    Item Remove();
    void Move(Item item, int queue);
    bool IsEmpty();
    void Apply(void (*func)(Item));
  private:
    int numQueues;
    List<Item> *list;
};


/* Constructor */
template <class Item>
PriorityQueue<Item>::PriorityQueue(int numQueues)
{
    PriorityQueue::numQueues = numQueues;
    list = new List<Item>;
}


/* Destructor */
template <class Item>
PriorityQueue<Item>::~PriorityQueue()
{
    delete list;
}


/* Agrega un item al final de la cola queue */
template <class Item>
void PriorityQueue<Item>::Append(Item item, int queue)
{
    ASSERT(queue >= 0 && queue < numQueues);
    list->SortedInsert(item, numQueues - queue);
}


/* Remueve el primer item de la cola con mayor prioridad y lo retorna */
template <class Item>
Item PriorityQueue<Item>::Remove()
{
    return list->Remove();
}


/* Mueve el item a la cola queue (si existe) */
template <class Item>
void PriorityQueue<Item>::Move(Item item, int queue)
{
    ASSERT(list->Delete(item));
    Append(item, queue);
}


/* Comprueba si todas las colas están vacía */
template <class Item>
bool PriorityQueue<Item>::IsEmpty()
{
    return list->IsEmpty();
}


/* "Aplica" la función func a cada item */
template <class Item>
void PriorityQueue<Item>::Apply(void (*func)(Item))
{
    list->Apply(func);
}


#endif // PRIORITYQUEUE_H

