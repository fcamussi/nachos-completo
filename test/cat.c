#include "utils.h"



/* Concatena los ficheros que se le pasan como argumentos y los muestra por la
   la consola */
int main(int argc, char *argv[])
{
    OpenFileId file;
    char buffer[100];
    int readed;
    int c;

    if (argc < 2)
    {
        print("Modo de uso: ");
        print(argv[0]);
        print(" ficheros\n");
        Exit(-1);
    }

    for (c = 1; c < argc; c++) // para cada fichero
    {
        file = Open(argv[c]);
        if (file < 0)
        {
            print("Error al abrir el fichero: ");
            print(argv[c]);
            print("\n");
            Exit(-1);
        }
        do
        {
            readed = Read(buffer, 100, file);
            Write(buffer, readed, ConsoleOutput);
        } while (readed > 0);
    }

    Exit(0);
}

