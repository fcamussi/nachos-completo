
/* Funciones útiles para el desarrollo de aplicaciones de espacio de usuario */


#ifndef UTILS_H
#define UTILS_H

#include "syscall.h"


#define NULL 0
#define TRUE 1
#define FALSE 0



/* strcmp */
int strcmp(const char *str1, const char *str2)
{
    while (*str1 == *str2)
    {
        if (*str1 == '\0')
            return *str1 - *str2;
        str1++;
        str2++;
    }
    return *str1 - *str2;
}


/* strlen */
int strlen(const char *str)
{
    int c = 0;

    while (*str != '\0')
    {
        str++;
        c++;
    }
    return c;
}


/* imprimo una cadena */
void print(const char *str)
{
    Write((char *)str, strlen(str), ConsoleOutput);
}


/* leo una linea, reemplazo el '\n' por un '\0' y retorno el tamaño */
int readLine(char *buffer, int size)
{
    int c;

    for (c = 0; c < size; c++)
    {
        Read(&buffer[c], 1, ConsoleInput);
        if (buffer[c] == '\n') break;
    }
    if (c == size) c--;
    buffer[c] = '\0';
    return c;
}


/* convierto un entero en una string y retorno el tamaño */
int intToStr(char *buffer, int n)
{
    int digit, c, i, neg;
    char tmp;

    // En caso de que sea negativo
    if (n < 0)
    {
        neg = 1;
        buffer[0] = '-';
        n = -n;
    }
    else neg = 0;

    // Convierto los dígitos en caracteres
    c = neg;
    do
    {
        digit = n % 10;
        n /= 10;
        buffer[c] = (char)digit + 48; // Sumo la posición del 0 en el ASCII
        c++;
    }
    while(n != 0);
    buffer[c] = '\0';

    // Los invierto para mostrarlos en orden
    c--;
    for (i = 0; i <= (c - neg) / 2; i++)
    {
        tmp = buffer[i + neg];
        buffer[i + neg] = buffer[c - i];
        buffer[c - i] = tmp;
    }

    return c + 1;
}


/* construyo un array de argumentos a partir de una cadena */
void buildArgv(char *buffer, char *argv[], int size)
{
    int i, c;

    c = 0;
    i = 0;
    do
    {
        while (buffer[i] == ' ') // ignoro los espacios
        {
            i++;
        }
        if (buffer[i] != '\n')
        {
            argv[c] = &(buffer[i]); // argumento c
            c++;
            i++;
            while (buffer[i] != ' ' && buffer[i] != '\n') // recorro la cadena
            {
                i++;
            }
            if (buffer[i] == '\n') break; // si terminó la linea salgo del while sin incrementar i
            buffer[i] = '\0';
            i++;
        }
    }
    while (c < size - 1 && buffer[i] != '\n');
    buffer[i] = '\0';
    argv[c] = NULL; // termino el array en NULL
}



#endif // UTILS_H

