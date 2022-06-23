#include "copy.h"
#include "system.h"



static void ReadMem(int addr, int size, int *value)
{
    if (! machine->ReadMem(addr, size, value))
#ifdef VM // reintento
    if (! machine->ReadMem(addr, size, value))
#endif
        ASSERT(false);
}


static void WriteMem(int addr, int size, int value)
{
    if (! machine->WriteMem(addr, size, value))
#ifdef VM // reintento
    if (! machine->WriteMem(addr, size, value))
#endif
        ASSERT(false);
}


/* Copia datos desde el espacio de usuario al espacio de kernel */
void CopyFromUser(int addr, int size, char *buffer)
{
    int to = addr + size;
    int tmp;
    int cant;

    while (addr < to)
    {
        cant = size >= 4 ? 4 : 1;
        ReadMem(addr, cant, &tmp);
        *(int *)buffer = (int)tmp;
        addr += cant;
        buffer += cant;
        size -= cant;
    }
}


/* Copia datos desde el espacio de kernel al espacio de usuario */
void CopyToUser(int addr, int size, char *buffer)
{
    int to = addr + size;
    int cant;

    while (addr < to)
    {
        cant = size >= 4 ? 4 : 1;
        WriteMem(addr, cant, (int)*(int *)buffer);
        addr += cant;
        buffer += cant;
        size -= cant;
    }
}


/* Copia una string desde el espacio de usuario al espacio de kernel */
void CopyStringFromUser(int addr, int size, char *buffer)
{
    int to = addr + size;
    int tmp;
    char *buf = buffer;
    
    while (addr < to)
    {
        ReadMem(addr, 1, &tmp);
        *buf = (char)tmp;
        if (*buf == '\0') break; // leo hasta el '\0'
        addr++;
        buf++;
    }
    // si no termina en '\0', entonces le ponemos el '\0' al final
    if (buffer[size - 1]) buffer[size - 1] = '\0';
}


/* Constructor de Arguments */
Arguments::Arguments()
{
    argc = 0;
    argv = NULL;
}


/* Destructor de Arguments */
Arguments::~Arguments()
{
    for (int c = 0; c < argc; c++)
        delete argv[c];
    delete [] argv;
}


/* Copia los argumentos desde el espacio de usuario al espacio de kernel
   (dentro del objeto) */
void Arguments::CopyFromUser(int addr)
{
    int a, tmp, size;
    char *buffer = new char[100];

    // cuantos argumentos hay?
    a = addr;
    ReadMem(a, 4, &tmp); // leemos argv[0]
    while (tmp != 0) // mientras sea != NULL
    {
        argc++;
        a += 4; // apunto al siguiente elemento de argv
        ReadMem(a, 4, &tmp); // leemos argv[argc]
    }

    argv = new char *[argc + 1]; // son argc argumentos + el NULL final

    a = addr;
    for (int c = 0; c < argc; c++)
    {
        ReadMem(a, 4, &tmp);
        CopyStringFromUser(tmp, 100, buffer); // leo el argumento
        size = strlen(buffer) + 1; // strlen excluye el '\0', entonces sumo 1
        argv[c] = new char[size];
        strcpy(argv[c], buffer);
        a += 4; // apunto al siguiente elemento de argv
    }
    argv[argc] = NULL; // el array de parámetros tiene que terminar en NULL

    delete [] buffer;
}


/* Copia los argumentos desde el espacio de kernel (desde el objeto)
   a la pila en el espacio de usuario
   El argumento sp es el stack pointer actual y la función retorna
   la posición actualizada (ya que la pila crece al inverso de la
   memoria) */
int Arguments::CopyToUserStack(int sp)
{
    int *newArgv;
    int size, fill;

    newArgv = new int[argc + 1]; // son argc argumentos + el NULL final

    for (int c = 0; c < argc; c++)
    {
        size = strlen(argv[c]) + 1; // strlen excluye el '\0', entonces sumo 1
        fill = 4 - size % 4; // cantidad de bytes de relleno
        sp -= (size + fill); // hago lugar para la cadena considerando la alineación
        CopyToUser(sp, size, argv[c]); // copio la cadena (cada char ocupa 1 byte)
        newArgv[c] = sp; // guardo la dirección de la cadena en el nuevo espacio
    }
    newArgv[argc] = 0; // el array de parámetros tiene que terminar en NULL

    // ahora tenemos que guardar el nuevo argv
    size = (argc + 1) * 4; // cada dirección de memoria ocupa 4 bytes
    sp -= size;
    CopyToUser(sp, size, (char *)newArgv);

    delete [] newArgv;

    return sp; // retorno el nuevo stack pointer
}


