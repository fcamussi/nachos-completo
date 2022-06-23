#include "utils.h"



/* echo */
int main(int argc, char *argv[])
{
    int c;

    for (c = 1; c < argc; c++)
    {
        print(argv[c]);
        print(" ");
    }
    print("\n");

    Exit(0);
}

