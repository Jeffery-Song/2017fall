#include<stdio.h>
int foo() {
    FILE* p;
    p = fopen("t.txt", "r");
    fclose(p);
    fclose(p);
    return 0;
}
