/* soma os elementos de um vetor
   comentario com mais de uma linha */
int soma(int v[], int n) {
    int i;
    int s;
    s = 0;
    i = 0;
    while (i < n) {
        s = s + v[i];
        i = i + 1;
    }
    return s;
}

void main(void) {
    int a[5];
    int total;
    total = soma(a, 5);
    output(total);
}
