#include <stdio.h>
int foo() {
    FILE* p;
    int i = 3;
    while (i * i < 36945616) {
        FILE* q = fopen("t.txt", "r");
        p = q;
        fclose(q);
        i++;
    }
    fgetc(p);
}
