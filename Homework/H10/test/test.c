void func() {
    while(1);
}
int main() {
    long a = 4;
    long* p = &a;
    p[a] = &func;
}
    
