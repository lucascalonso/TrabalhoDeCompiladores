/* programa fatorial em C- */
int fatorial(int n) {
    if (n <= 1)
        return 1;
    else
        return n * fatorial(n - 1);
}

void main(void) {
    int x;
    x = input();
    output(fatorial(x));
}
