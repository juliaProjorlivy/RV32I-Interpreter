struct Fib
{
    int a;
    int b;
};
int fib(int n, struct Fib pair)
{
    if (n == 0) return pair.a;
    else if(n == 1) return pair.b;
    else
    {
        struct Fib out = {pair.b, pair.a + pair.b};
        return fib(n - 1, out);
    };
}

int main(int argc, char *argv[])
{
    int n = 0;
    if(argc < 2) {n = 5;}
    else {n = argv[1][0] + '0';}
    struct Fib initial_pair = {0, 1};
    volatile int result = fib(n, initial_pair);
    return result;
}

