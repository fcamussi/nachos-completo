#include "utils.h"

#define BUFFER 100
#define ARGV 10


/* Un shell básico */
int main()
{
    SpaceId newProc;
    int retValue = 0;
    char *welcome = "Bienvenido al Shell de NachOS™\n";
    char *prompt = "% ";
    char buffer[BUFFER];
    char *buffer2;
    int i, readed;
    char *argv[ARGV]; // acá coloco los argumentos
    int bg = 0;


    print(welcome);
    while(1)
    {
        print(prompt);
        readed = readLine(buffer, BUFFER);
        if(readed > 0)
        {
            if (strcmp(buffer, "exit") == 0)
            {
                print("Chau!\n");
                Exit(0);
            }
            else if (strcmp(buffer, "ret") == 0) // ver valor de retorno
            {
                i = intToStr(buffer, retValue);
                buffer[i] = '\n';
                buffer[i + 1] = '\0';
                print(buffer);
            }
            else
            {
                buffer[readed] = '\n'; // marco el final de la cadena

                // me fijo si al principio de la linea hay un ampersand
                buffer2 = buffer;
                while (*buffer2 == ' ') buffer2++, readed--;
                if (*buffer2 == '&')
                {
                    bg = 1;
                    buffer2++;
                }

                buildArgv(buffer2, argv, ARGV); // construyo argv a partir de la linea leida

                if (argv[0] == NULL) continue; // nada que ejecutar

                if (bg) // background
                {
                    newProc = Exec(argv[0], argv, FALSE); // argv[0] es el nombre del ejecutable
                    bg = 0;
                }
                else // foreground
                {
                    newProc = Exec(argv[0], argv, TRUE);
                    retValue = Join(newProc);
                }
            }
        }
    }
}

