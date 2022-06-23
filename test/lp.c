#include "utils.h"



void printHeader()
{
    print("ID\tNOMBRE\t\tESTADO\n");
}


void printProc(struct LP *lp)
{
    char *status[] = { "just_created", "running", "ready", "blocked" };
    char buffer[100];
    int i;

    i = intToStr(buffer, lp->id);
    buffer[i] = '\0';
    print(buffer);
    print("\t");
    print(lp->name);
    print("\t\t");
    print(status[lp->status]);
    print("\n");
}


int main()
{
    struct LP lp;
    int lpValue;

    printHeader();

    lpValue = ListProc(0, &lp);
    do
    {
        printProc(&lp);
        lpValue = ListProc(lpValue, &lp);
    }
    while (lpValue > 0);

    Exit(0);
}

