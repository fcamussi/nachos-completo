#include "syscall.h"

#define N 25



int fibonacci(int n)
{
    if (n == 0 || n == 1) return n;
    else return fibonacci(n - 1) + fibonacci(n - 2);
}


int main()
{
    Exit(fibonacci(N));
}

