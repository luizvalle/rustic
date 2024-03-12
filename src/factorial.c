#include <stdio.h>

int factorial(int n)
{
    if (n <= 1) {
        return 1;
    }

    return factorial(n - 1) * n;
}

int main(int argc, char **argv)
{
    int n = 5;

    printf("%d! = %d\n", n, factorial(n));

    return 0;
}
