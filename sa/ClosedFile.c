#include <stdio.h>
int foo() {
    FILE* p;
    p = fopen("t.txt", "r");
    fgetc(p);
    fclose(p);
    fgetc(p);
}
