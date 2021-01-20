#include <stdio.h>

int hello() {
    printf("Hello, world!\n");
    return 0;
}

int ret33() {
    return 33;
}

int multi2(int a) {
    return a * 2;
}

int sum(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

int print2num(int a, int b) {
    printf("a = %d, b = %d\n", a, b);
    return 0;
}

int out(int n) {
    printf("%d\n", n);
    return 0;
}
