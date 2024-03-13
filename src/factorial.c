#include <stdio.h>

void foo() {
}

int factorial(int n)
{
    if (n <= 1) {
        return 1;
    }

    foo();

    return factorial(n - 1) * n;
}

int main(int argc, char **argv)
{
    int n = 10;

    printf("%d! = %d\n", n, factorial(n));

    return 0;
}
