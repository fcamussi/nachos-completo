#include "utils.h"


/* Copia el fichero file1 en un nuevo fichero file2 */
int main(int argc, char *argv[])
{
    OpenFileId file1, file2;
    char buffer[100];
    int readed;

    if (argc < 3)
    {
        print("Modo de uso: ");
        print(argv[0]);
        print(" file1 file2\n");
        Exit(-1);
    }

    file1 = Open(argv[1]);
    if (file1 < 0)
    {
        print("Error al abrir el fichero: ");
        print(argv[1]);
        print("\n");
        Exit(-1);
    }

    Create(argv[2]);
    file2 = Open(argv[2]);
    if (file2 < 0)
    {
        print("Error al crear el fichero: ");
        print(argv[2]);
        print("\n");
        Exit(-1);
    }

    // copiamos
    do
    {
        readed = Read(buffer, 100, file1);
        Write(buffer, readed, file2);
    } while (readed > 0);

    Exit(0);
}

