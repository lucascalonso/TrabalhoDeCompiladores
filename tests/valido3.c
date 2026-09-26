int mdc(int u, int v) {
    if (v == 0) return u;
    else return mdc(v, u - u / v * v);
}
