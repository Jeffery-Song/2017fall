#include <stdio.h>
extern int *buf;
int k[1] = {100};
int *p = k;
int main() {
    printf("%d, %d\n", k, p);
    printf("%d\n", buf);
}
