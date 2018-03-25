int fac(int n) {
    int ret = 1;
    int i = 1;
    while(i) {
        if (i > n) break;
        ret *= i;
        i++;
    }
    return ret;
}
